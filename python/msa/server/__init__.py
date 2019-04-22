from msa.version import v as msa_version

import aiohttp
from aiohttp import web

routes = web.RouteTableDef()

@routes.get("/ping")
async def ping_handler(request):
    return web.Response(text="pong")

@routes.get("/version")
async def version_handler(request):
    return web.Response(text=msa_version)


def start_server():
    app = web.Application()
    app.add_routes(routes)
    web.run_app(app)
