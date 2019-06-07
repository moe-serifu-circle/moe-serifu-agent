from aiohttp import web

from msa.version import v as msa_version
from msa.builtins.tty.events import TextInputEvent


def register_default_routes(route_adapter):

    @route_adapter.get("/ping")
    async def ping_handler(request, raw_data=None):
        return web.Response(text="pong")

    @route_adapter.get("/version")
    async def version_handler(request, raw_data=None):
        return web.Response(text=msa_version)

    @route_adapter.post("/remote_command")
    async def echo_handler(request, raw_data=None):
        message = dict(await request.post())

        evt = TextInputEvent()
        evt.init(message)
        request.app["supervisor"].fire_event(evt)

        return web.Response(text="Triggering echo")
