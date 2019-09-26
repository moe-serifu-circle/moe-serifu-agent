import enum

from msa.api.patcher import ApiPatcher
from msa.api.context import ApiContext
from msa.api import api_clients 




def get_api(context, **kwargs):
    """
        kwargs should only be provided if the context you are attempting to retrieve has not been loaded/patched yet.
    """
    if context is None:
        raise Exception("get_api: context cannot be None.")

    if context == ApiContext.local:
        api_client = api_clients.ApiLocalClient(**kwargs)

    elif context == ApiContext.rest:
        api_client = api_clients.ApiRestClient(**kwargs)

    elif context == ApiContext.websocket:
        api_client = api_clients.ApiWebsocketClient(**kwargs)

    if len(**kwargs.keys()) == 0:
        api_instance = ApiPatcher.load(context, api_client)
    else:
        api_instance = ApiPatcher.load(context)

    return api_instance



