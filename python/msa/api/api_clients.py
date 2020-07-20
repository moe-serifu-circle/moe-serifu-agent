#!/usr/bin/env python
# -*- coding: utf-8 -*-
from functools import partial
import signal
import requests
import time
import asyncio
import aiohttp
import json
import traceback

from msa.core.event import Event
from msa.server.server_request import SeverRequest
from msa.server.url_param_parser import UrlParamParser


class ApiResponse:
    def __init__(self, status, raw=None, payload=None):
        self.status = status
        self.raw = raw
        self.payload = payload

        if self.raw is not None:
            if isinstance(raw, str):
                self.text = raw
            else:
                self.text = raw.decode("utf-8"())
        else:
            self.raw = ""
            self.text = ""

    @property
    def json(self):
        if self.payload:
            return self.payload
        elif self.raw:
            try:
                return json.loads(self.text)
            except:
                return None
        else:
            return {}


class ApiRestClient:

    def __init__(self, host="localhost", port=8080):

        self.host = host
        self.port = port
        self.base_url = "http://{}:{}".format(self.host, self.port)

        self.session = None

    async def connect(self):  # pragma: no coverage
        self.session = aiohttp.ClientSession()

    async def disconnect(self):  # pragma: no coverage
        await self.session.close()
        self.session = None

    async def _wrap_api_call(self, func, endpoint, payload=None):

        async with  func(self.base_url + endpoint, json=payload) as response:
            raw_text = await response.text()
            return ApiResponse(response.status, raw=raw_text)

    async def get(self, endpoint):
        return await self._wrap_api_call(self.session.get, endpoint)

    async def post(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.post, endpoint, payload)

    async def put(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.put, endpoint, payload)
    
    async def delete(self, endpoint, payload=None):
        return await self._wrap_api_call(self.session.delete, endpoint, payload)

class ApiWebsocketClient:

    def __init__(self, loop, interact, propagate, host="localhost", port=8080):
        """

        :param loop:
        :param interact:
        :param propagate:
        :param host:
        :param port:
        """
        self.loop = loop
        self.host = host
        self.port = port
        self.interact = interact
        self.propagate = propagate
        self.base_url = "http://{}:{}/ws".format(self.host, self.port)

        self.message_buffer = asyncio.Queue()

        self.queue = asyncio.Queue()
        self.propagate_queue = asyncio.Queue()
        self.client_id = None

        self.client_session = None
        self.ws = None

    async def connect(self):
        async with aiohttp.ClientSession() as session:
            self.client_session = session
            async with session.ws_connect(self.base_url) as ws:
                self.ws = ws

                self.loop.create_task(self.interact())

                async def prop():
                    await self.propagate(self.propagate_queue)
                self.loop.create_task(prop())

                async for msg in ws:
                    if msg.type == aiohttp.WSMsgType.TEXT:
                        data = json.loads(msg.data)

                        if data["type"] == "response":
                            response = ApiResponse("success", payload=data["payload"])

                        elif data["type"] == "event_propagate":
                            new_event = Event.deserialize(data["payload"])
                            new_event.network_propagate = False
                            if new_event.propagate_target == self.client_id or new_event.propagate_target == "all":
                                self.propagate_queue.put_nowait(new_event)
                            continue

                        elif data["type"] == "empty_response":
                            pass

                        elif data["type"] == "notify_id":
                            self.client_id = data["payload"]["id"]
                            continue
                        elif data["type"] == "error":
                            response = ApiResponse("failed", payload=data["payload"])
                        else:
                            raise Exception("Bad payload response:", data)

                        self.queue.put_nowait(response)

    async def disconnect(self):  # pragma: no coverage
        if self.ws:
            await self.ws.close()

        if self.client_session:
            await self.client_session.close()

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
    
    async def delete(self, endpoint, payload=None):
        return await self._wrap_api_call("delete", endpoint, payload)


class ApiLocalClient(dict):
    def __init__(self, loop):
        self.loop = loop

        from msa.server import route_adapter_instance
        self.route_adapter = route_adapter_instance
        self.client = self

    async def _call_api_route(self, verb, route, payload=None):
        func, url_params = self.route_adapter.lookup_route_and_resolve_url_params(verb, route)
        if func is None:
            raise Exception(f"{self.__class__.__name__}: no api route {verb}:{route} exists.")

        if not callable(func):
            raise Exception(f"{self.__class__.__name__}: api route is not callable: {func}")

        request = SeverRequest("local", verb, route, payload, url_params)

        try:
            result_payload = await func(request)
            return ApiResponse("success", payload=result_payload)
        except Exception as e:
            return ApiResponse("failed", raw=traceback.format_exc())

    async def get(self, route):
        return await self._call_api_route("get", route)

    async def post(self, route, payload=None):
        return await self._call_api_route("post", route, payload=payload)

    async def put(self, route, payload=None):
        return await self._call_api_route("put", route, payload=payload)

    async def delete(self, route, payload=None):
        return await self._call_api_route("delete", route, payload=payload)
        






