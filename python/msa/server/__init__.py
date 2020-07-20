import asyncio
from aiohttp import WSCloseCode

from msa.data import start_db_engine, stop_db_engine
from msa.server.route_adapter import RouteAdapter
from msa.server.default_routes import register_default_routes
from msa.server.event_propagate import EventPropagationRouter

from aiohttp import web

# create global route adapter
route_adapter_instance = RouteAdapter()
register_default_routes(route_adapter_instance)


async def start_supervisor(app):
    app["supervisor"].start()


async def stop_supervisor(app):
    app["supervisor"].logger.info("*** trigger shutdown")
    await app["supervisor"].exit()


async def on_shutdown(app):
    for ws in set(app["websockets"]):
        await ws.close(code=WSCloseCode.GOING_AWAY, message="Server shutdown")


def start_server(config_context):
    app = web.Application()

    event_propagation_router = EventPropagationRouter()

    app["config_context"] = config_context

    # get loop and db
    loop = asyncio.get_event_loop()

    # init supervisor
    from msa import core as msa_core
    from msa.core.supervisor import Supervisor

    supervisor = Supervisor()
    msa_core.supervisor = supervisor
    app["supervisor"] = supervisor
    supervisor.init(loop, app["config_context"], route_adapter_instance)
    loop.run_until_complete(start_db_engine())

    event_propagation_router.register_propagate_subscription(supervisor.event_bus)

    # register routes
    route_adapter_instance.register_app(app)
    app.add_routes(route_adapter_instance.get_route_table())

    # startup hooks
    app.on_startup.append(event_propagation_router.app_start)
    app.on_startup.append(start_supervisor)

    # onshutdown
    app.on_shutdown.append(on_shutdown)

    # cleanup routes
    app.on_cleanup.append(stop_supervisor)
    app.on_cleanup.append(event_propagation_router.app_stop)
    app.on_cleanup.append(stop_db_engine)

    try:
        import aiomonitor

        host, port = "localhost", 8080
        locals_ = {"port": port, "host": host}
        with aiomonitor.start_monitor(loop=loop, locals=locals_):
            # run application with built in aiohttp run_app function

            try:
                web.run_app(app, port=port, host=host)
            except:
                pass
            finally:
                loop.close()

    except:
        try:
            web.run_app(app)
        except:
            pass
        finally:
            loop.close()
