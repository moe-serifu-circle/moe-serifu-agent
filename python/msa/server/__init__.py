from msa.core import RunMode, supervisor
from msa.data import start_db_engine
from msa.server.route_adapter import RouteAdapter
from msa.server.default_routes import register_default_routes

from aiohttp import web

# create global route adapter
route_adapter = RouteAdapter()
register_default_routes(route_adapter)


async def start_supervisor(app):
    supervisor.init(app.loop, app["config_context"], app["db"])
    supervisor.start()
    app["supervisor"] = supervisor

async def stop_supervisor(app):
    app["supervisor"].logger.info("*** trigger shutdown")
    await app["supervisor"].exit()


def start_server(config_context):
    app = web.Application()

    app["config_context"] = config_context

    app.add_routes(route_adapter.get_route_table())
    app.on_startup.append(start_db_engine)
    app.on_startup.append(start_supervisor)
    app.on_cleanup.append(stop_supervisor)
    web.run_app(app)
