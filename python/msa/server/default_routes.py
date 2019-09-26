from msa.version import v as msa_version


def register_default_routes(route_adapter):

    @route_adapter.get("/ping")
    def ping_handler(request=None, raw_data=None):
        return {"text": "pong"}

    @route_adapter.get("/version")
    def version_handler(request=None, raw_data=None):
        return {"text": msa_version}


