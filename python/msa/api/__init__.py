import enum
import asyncio

from msa.api.patcher import ApiPatcher
from msa.api.context import ApiContext
from msa.api import api_clients 




def get_api(context, plugin_whitelist=None, **kwargs):
    """
        kwargs should only be provided if the context you are attempting to retrieve has not been loaded/patched yet.
    """
    if context is None:
        raise Exception("get_api: context cannot be None.")


    if len(kwargs.keys()) == 0:
        api_instance = ApiPatcher.load(context)
    else:
        if context == ApiContext.local:
            api_client = api_clients.ApiLocalClient(**kwargs)

        elif context == ApiContext.rest:
            api_client = api_clients.ApiRestClient(**kwargs)

        elif context == ApiContext.websocket:
            api_client = api_clients.ApiWebsocketClient(**kwargs)
            
        if plugin_whitelist is None:
            raise Exception("get_api: plugin_whitelist cannot be none when **kwargs are provided to get_api.")

        api_instance = ApiPatcher.load(context, api_client, plugin_whitelist)

    return api_instance


def run_async(coroutine):
    loop = asyncio.get_event_loop()
    if loop.is_running():
        raise Exception("Asyncio event loop cannot be running in order to use run_async helper function.")
    return loop.run_until_complete(coroutine)
