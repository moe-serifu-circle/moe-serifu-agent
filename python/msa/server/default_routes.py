from aiohttp import web

from msa.version import v as msa_version
from msa.core import supervisor


def register_default_routes(route_adapter):

    @route_adapter.get("/ping")
    async def ping_handler(request=None, raw_data=None):
        return {"text": "pong"}

    @route_adapter.get("/version")
    async def version_handler(request=None, raw_data=None):
        return {"text": "msa_version"}


    @route_adapter.post("/scripting/script")
    async def add_script(request=None, raw_data=None):

        if raw_data:
            data = raw_data
        else:
            data = await dict(request.post())

        from msa.builtins.scripting.events import AddScriptEvent
        new_event = AddScriptEvent().init(data)
        supervisor.fire_event(new_event)
        


        











