from functools import partial
import requests
import time

from msa.api.base_methods import register_base_methods
from msa.server import route_adapter
from msa.server.default_routes import register_default_routes



class MsaApi(dict):
    def __init__(self, *args, **kwargs):
        super(MsaApi, self).__init__(*args, **kwargs)
        self.__dict__ = self


class MsaApiRestClient:

    def __init__(self, host="localhost", port=8080, script_mode=False):

        self.host = host
        self.port = port
        self.base_url = "http://{}:{}".format(self.host, self.port)

    def _wrap_api_call(self, func, endpoint, **kwargs):
        n = 0
        fail = 3
        while True:
            try:
                return func(self.base_url + endpoint,  **kwargs)
            except requests.exceptions.ConnectionError:
                n += 1

                if n == 1:
                    print("This is taking longer than expected, we seem to be having some connection troubles. Trying again.")
                elif n == 2:
                    print("Hmm, something must be up.")

            print("Unfortunately, I was unable to read the msa daemon instance at {}".format(self.base_url))
            print("Please check your connection and try again")
            return None

    def get(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.get, endpoint, **kwargs)

    def post(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.post, endpoint, **kwargs)

    def put(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.put, endpoint, **kwargs)
    
    def update(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.update, endpoint, **kwargs)
    
    def delete(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.delete, endpoint, **kwargs)


class MsaApiWrapper:
    def __init__(self, host="localhost", port=8080, script_mode=False):

        self.api = MsaApi()
        self.api.script_mode = script_mode
        self.api.rest_client = MsaApiRestClient()

        self._registration_frozen = False

        register_base_methods(self)

        self._registration_frozen = True


    def register_method(self):
        if not self._registration_frozen:
            def decorator(func):
                self.api[func.__name__] = partial(func, self.api)
            return decorator
        else:
            raise Exception(f"MsaAPI Method Registration is frozen, failed to register method: {func.__name__}")

    def get_api(self):
        return self.api


class MsaServerApi:
    def __init__(self, database):
        register_default_routes(route_adapter)
        self.route_adapter = route_adapter
        self.database = database

    def _call_api_route(self, verb, route, payload=None):
        func = self.route_adapter.lookup_route(verb, route)
        if func is None:
            raise Exception(f"{self.__cls__.__name}: no api route {verb}:{route} exists.")

        if payload is not None:
            func(None, raw_data=payload, database=self.database)
        else:
            func(None)

    def get(self, route):
        self._call_api_route("get", route)

    def post(self, route, payload=None):
        self._call_api_route("post", route, payload=None)

    def put(self, route, payload=None):
        self._call_api_route("put", route, payload=None)

    def delete(self, route, payload=None):
        self._call_api_route("delete", route, payload=None)

class MsaLocalApiWrapper:
    def __init__(self, database):
        self.api = MsaServerApi(database)

        self._registration_frozen = False
        register_base_methods(self)
        self._registration_frozen = True

    def register_method(self):
        if not self._registration_frozen:
            def decorator(func):
                self.api[func.__name__] = partial(func, self.api)
            return decorator
        else:
            raise Exception(f"MsaServerAPI Method Registration is frozen, failed to register method: {func.__name__}")

    def get_api(self):
        return self.api






