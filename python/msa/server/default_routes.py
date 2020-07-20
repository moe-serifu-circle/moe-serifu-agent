from msa.version import v as msa_version


def register_default_routes(route_adapter):

    @route_adapter.get("/ping")
    async def ping_handler(request=None, raw_data=None):
        return {"response_status": "success", "text": "pong"}

    @route_adapter.get("/version")
    async def version_handler(request=None, raw_data=None):
        return {"response_status": "success", "text": msa_version}


