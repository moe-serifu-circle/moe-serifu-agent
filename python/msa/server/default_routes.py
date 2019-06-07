from aiohttp import web

from msa.version import v as msa_version

def register_default_routes(route_adapter):

    @route_adapter.get("/ping")
    async def ping_handler(request, raw_data=None):
        return web.Response(text="pong")

    @route_adapter.get("/version")
    async def version_handler(request, raw_data=None):
        return web.Response(text=msa_version)

