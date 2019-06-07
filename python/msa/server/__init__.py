from msa.core import RunMode, supervisor
from msa.data import start_db_engine
from msa.server.route_adapter import RouteAdapter
from msa.server.default_routes import register_default_routes

import aiohttp
from aiohttp import web
import json

# create global route adapter
route_adapter = RouteAdapter()
register_default_routes(route_adapter)


async def start_supervisor(app):
    supervisor.init(app.loop, {"log_level": "info", "config_file": "msa_config.json"})
    supervisor.start()
    app["supervisor"] = supervisor


def start_server(config_context):
    app = web.Application()
    app.add_routes(route_adapter.get_route_table())
    app.on_startup.append(start_db_engine)
    app.on_startup.append(start_supervisor)
    web.run_app(app)
