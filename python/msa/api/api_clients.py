#!/usr/bin/env python
# -*- coding: utf-8 -*-
from functools import partial
import requests
import time
import asyncio
import aiohttp

from msa.server import route_adapter


class ApiRestClient:

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
        print(kwargs)
        return self._wrap_api_call(requests.post, endpoint, **kwargs)

    def put(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.put, endpoint, **kwargs)
    
    def update(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.update, endpoint, **kwargs)
    
    def delete(self, endpoint, **kwargs):
        return self._wrap_api_call(requests.delete, endpoint, **kwargs)

class ApiWebsocketClient:

    def __init__(self, loop, interact, host="localhost", port=8080):
        self.loop = loop
        self.host = host
        self.port = port
        self.base_url = "http://{}:{}".format(self.host, self.port)

        self.interact = interact

        self.message_buffer = asyncio.Queue()

    async def _connect(self):
        async with aiohttp.ClientSession() as session:
            async with session.ws_connect('http://localhost:8080') as ws:
                self.ws = ws
                await self.interact(self)

    async def _wrap_api_call(self, verb, endpoint, data):

        payload = {
            "verb": verb,
            "route": "/ws" + endpoint,
            "data":  data
            
        }
        await self.ws.send_json(payload)
        await self.ws.receive_str()

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


class ApiLocalClient(dict):
    def __init__(self, loop):
        super(MsaApiServerClient, self).__init__()
        self.__dict__ = self
        self.loop = loop

        self.route_adapter = route_adapter
        self.client = self

    def _call_api_route(self, verb, route, payload=None):
        func = self.route_adapter.lookup_route(verb, route)
        if func is None:
            raise Exception(f"{self.__class__.__name__}: no api route {verb}:{route} exists.")

        if not callable(func):
            raise Exception(f"{self.__class__.__name__}: api route is not callable: {func}")

        if payload is not None:
            func(payload)
        else:
            func(None)

    def get(self, route):
        self._call_api_route("get", route)

    def post(self, route, data=None, json=None):
        self._call_api_route("post", route, payload=data or json)

    def put(self, route, data=None, json=None):
        self._call_api_route("put", route, payload=data or json)

    def delete(self, route, data=None, json=None):
        self._call_api_route("delete", route, payload=data or json)
        






