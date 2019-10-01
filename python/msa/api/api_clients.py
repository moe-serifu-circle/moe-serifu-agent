#!/usr/bin/env python
# -*- coding: utf-8 -*-
from functools import partial
import signal
import requests
import time
import asyncio
import aiohttp
import json

from msa.server import route_adapter


class ApiResponse:
    def __init__(self, status_code, raw):
        self.status_code = status_code
        self.raw = raw

        if isinstance(raw, str):
            self.text = raw
        else:
            self.text = raw.decode("utf-8"())

    def json(self):
        return json.loads(self.text)

class ApiRestClient:

    def __init__(self, host="localhost", port=8080):

        self.host = host
        self.port = port
        self.base_url = "http://{}:{}".format(self.host, self.port)

        self.session = None

    async def connect(self):
        self.session = aiohttp.ClientSession()

    async def disconnect(self):
        await self.session.close()
        self.session = None

    async def _wrap_api_call(self, func, endpoint, payload=None):

        async with  func(self.base_url + endpoint, json=payload) as response:
            raw_text = await response.text()
            return ApiResponse(response.status, raw_text)

    async def get(self, endpoint):
        return await self._wrap_api_call(self.session.get, endpoint)

    async def post(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.post, endpoint, payload)

    async def put(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.put, endpoint, payload)
    
    async def update(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.update, endpoint, payload)
    
    def delete(self, endpoint, payload=None):
        return self._wrap_api_call(self.session.delete, endpoint, payload)

class ApiWebsocketClient:

    def __init__(self, loop, interact, host="localhost", port=8080):
        self.loop = loop
        self.host = host
        self.port = port
        self.base_url = "http://{}:{}/ws".format(self.host, self.port)

        self.interact = interact

        self.message_buffer = asyncio.Queue()

        self.queue = asyncio.Queue()
        self.state = {}

    async def _connect(self):
        async with aiohttp.ClientSession() as session:
            async with session.ws_connect(self.base_url) as ws:
                self.ws = ws
                self.loop.create_task(self.interact(self.api, self.state))

                async for msg in ws:
                    if msg.type == aiohttp.WSMsgType.TEXT:
                        self.queue.put_nowait(
                            ApiResponse(200, msg.data))


    def start(self):

        try:
            self.loop.run_until_complete(self._connect())
        except KeyboardInterrupt:
            print(1)
            self._stop()
            print(2)
        except asyncio.CancelledError:
            pass
            
    async def stop(self):
        await self.ws.close()

    async def _wrap_api_call(self, verb, endpoint, payload):

        wrapped_payload = {
            "verb": verb,
            "route": endpoint,
            "data":  payload
            
        }
        await self.ws.send_json(wrapped_payload)
        response = await self.queue.get()
        return response

    async def get(self, endpoint):
        return await self._wrap_api_call("get", endpoint, None)

    async def post(self, endpoint, payload=None):
        return await self._wrap_api_call("post", endpoint, payload)

    async def put(self, endpoint, payload=None):
        return await self._wrap_api_call("put", endpoint, payload)
    
    async def update(self, endpoint, payload=None):
        return await self._wrap_api_call("update", endpoint, payload)
    
    async def delete(self, endpoint, payload=None):
        return await self._wrap_api_call("delete", endpoint, payload)


class ApiLocalClient(dict):
    def __init__(self, loop):
        self.loop = loop

        self.route_adapter = route_adapter
        self.client = self

    async def _call_api_route(self, verb, route, payload=None):
        func = self.route_adapter.lookup_route(verb, route)
        if func is None:
            raise Exception(f"{self.__class__.__name__}: no api route {verb}:{route} exists.")

        if not callable(func):
            raise Exception(f"{self.__class__.__name__}: api route is not callable: {func}")

        if payload is not None:
            return await func(payload)
        else:
            return await func(None)

    async def get(self, route):
        return await self._call_api_route("get", route)

    async def post(self, route, payload=None):
        return await self._call_api_route("post", route, payload=payload)

    async def put(self, route, payload=None):
        return await self._call_api_route("put", route, payload=payload)

    async def delete(self, route, payload=None):
        return await self._call_api_route("delete", route, payload=payload)
        






