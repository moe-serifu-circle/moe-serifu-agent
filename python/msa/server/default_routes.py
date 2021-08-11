from msa.version import v as msa_version

from msa.server.server_response import ServerResponseText, ServerResponseType


def register_default_routes(route_adapter):
    @route_adapter.get("/ping")
    async def ping_handler(request=None, raw_data=None):
        return ServerResponseText(ServerResponseType.success, text="pong")

    @route_adapter.get("/version")
    async def version_handler(request=None, raw_data=None):
        return ServerResponseText(ServerResponseType.success, text=msa_version)
