import json
import unittest
from unittest.mock import patch, MagicMock
import asyncio
from schema import Schema, And

import aiohttp
from aiohttp.test_utils import TestClient, TestServer
from aiohttp import web

from msa.api.api_clients import (
    ApiRestClient,
    ApiWebsocketClient,
    ApiLocalClient,
    ApiResponse,
)
from msa.core.event import Event
from msa.server.server_response import ServerResponseJson, ServerResponseType


def run_aiohttp_client_test(app, func):
    loop = asyncio.get_event_loop()

    async def get_client():
        return TestClient(TestServer(app), loop=loop)

    client = loop.run_until_complete(get_client())

    loop.run_until_complete(client.start_server())
    loop.run_until_complete(func(loop, client))
    loop.run_until_complete(client.close())


class LocalClientTest(unittest.TestCase):
    def setUp(self) -> None:
        fake_loop = MagicMock()
        self.api_client = ApiLocalClient(fake_loop)

    def test_get(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        async def get_func(request):
            return ServerResponseJson(ServerResponseType.success, {"sum": 3 + 3})

        lut_fun.return_value = (get_func, {})

        result = loop.run_until_complete(self.api_client.get("/test"))
        self.assertEqual({"sum": 6}, result.json)

    def test_post(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        async def post_func(request):
            return ServerResponseJson(ServerResponseType.success, request.data)

        lut_fun.return_value = (post_func, {})

        request_payload = {"verb": "post", "key": 1}
        result = loop.run_until_complete(
            self.api_client.post("/test", payload=request_payload)
        )
        self.assertEqual(request_payload, result.json)

    def test_put(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        async def put_func(request):
            return ServerResponseJson(ServerResponseType.success, request.data)

        lut_fun.return_value = (put_func, {})

        request_payload = {"verb": "put", "key": 1}
        result = loop.run_until_complete(
            self.api_client.put("/test", payload=request_payload)
        )
        self.assertEqual(request_payload, result.json)

    def test_delete(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        async def delete_func(request):
            return ServerResponseJson(ServerResponseType.success, request.data)
            # return {"verb": "delete", "payload": request.data}

        lut_fun.return_value = (delete_func, {})

        request_payload = {"verb": "delete", "key": 1}
        result = loop.run_until_complete(
            self.api_client.delete("/test", payload=request_payload)
        )
        self.assertEqual(request_payload, result.json)

    def test_non_existant_route(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        lut_fun.return_value = (None, {})

        with self.assertRaises(Exception) as cm:
            loop.run_until_complete(self.api_client.get("/test"))

        self.assertEqual(
            "ApiLocalClient: no api route get:/test exists.", str(cm.exception)
        )

    def test_invalid_route(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        lut_fun.return_value = (5, {})

        with self.assertRaises(Exception) as cm:
            loop.run_until_complete(self.api_client.get("/test"))

        self.assertEqual(
            "ApiLocalClient: api route is not callable: 5", str(cm.exception)
        )

    def test_throwing_exception(self):
        loop = asyncio.get_event_loop()
        route_adapter_mock = MagicMock()
        lut_fun = MagicMock()
        route_adapter_mock.lookup_route_and_resolve_url_params = lut_fun

        self.api_client.route_adapter = route_adapter_mock

        async def func(request):
            raise Exception("my_exception")

        lut_fun.return_value = (func, {})

        result = loop.run_until_complete(self.api_client.get("/test"))

        self.assertTrue("Exception: my_exception" in result.text)


class RestClientTest(unittest.TestCase):
    def get_application(self):
        # set up server
        async def get_test(request):
            return web.json_response(
                {"status": "success", "text": "get_text", "payload": None}
            )

        async def post_test(request):
            data = await request.json()
            return web.json_response(data)

        async def put_test(request):
            data = await request.json()
            return web.json_response(data)

        async def delete_test(request):
            data = await request.json()
            return web.json_response(data)

        app = web.Application()
        app.router.add_get("/test", get_test)
        app.router.add_post("/test", post_test)
        app.router.add_put("/test", put_test)
        app.router.add_delete("/test", delete_test)
        return app

    def test_get(self):
        async def async_test_get(loop, client):

            api_client = ApiRestClient(host="localhost", port=8080)
            api_client.session = client
            api_client.base_url = ""

            result = await api_client.get("/test")
            self.assertEqual("success", result.status)
            self.assertEqual("get_text", result.text)
            self.assertEqual(None, result.json)

        run_aiohttp_client_test(self.get_application(), async_test_get)

    def test_post(self):
        async def async_test_post(loop, client):
            api_client = ApiRestClient(host="localhost", port=8080)
            api_client.session = client
            api_client.base_url = ""

            payload = {"status": "success", "text": "test text", "payload": None}

            result = await api_client.post("/test", payload=payload)
            self.assertEqual(payload["status"], result.status)
            self.assertEqual(payload["text"], result.text)
            self.assertEqual(payload["payload"], result.json)

        run_aiohttp_client_test(self.get_application(), async_test_post)

    def test_put(self):
        async def async_test_put(loop, client):
            api_client = ApiRestClient(host="localhost", port=8080)
            api_client.session = client
            api_client.base_url = ""

            payload = {"status": "success", "text": "test text", "payload": None}

            result = await api_client.put("/test", payload=payload)
            self.assertEqual(payload["status"], result.status)
            self.assertEqual(payload["text"], result.text)
            self.assertEqual(payload["payload"], result.json)

        run_aiohttp_client_test(self.get_application(), async_test_put)

    def test_delete(self):
        async def async_test_delete(loop, client):

            api_client = ApiRestClient(host="localhost", port=8080)
            api_client.session = client
            api_client.base_url = ""

            payload = {"status": "success", "text": "test text", "payload": None}

            result = await api_client.delete("/test", payload=payload)
            self.assertEqual(payload["status"], result.status)
            self.assertEqual(payload["text"], result.text)
            self.assertEqual(payload["payload"], result.json)

        run_aiohttp_client_test(self.get_application(), async_test_delete)


class WebsocketClientTest(unittest.TestCase):
    def get_application(self):
        async def ws_handler(request):
            ws = web.WebSocketResponse()
            await ws.prepare(request)

            await ws.send_str(
                json.dumps({"type": "notify_id", "payload": {"id": "123"}})
            )

            # propagate an event to all
            fake_event = NetworkPropDummyEvent().init({"prop_1": 1, "prop_2": "asdf"})
            fake_event.network_propagate()
            fake_event.target("all")
            fake_event_data = fake_event.get_metadata()
            await ws.send_str(
                json.dumps({"type": "event_propagate", "payload": fake_event_data})
            )

            # propagate an event to other client
            fake_event = NetworkPropDummyEvent().init({"prop_1": 2, "prop_2": "asdf2"})
            fake_event.network_propagate()
            fake_event.target("bob")
            fake_event_data = fake_event.get_metadata()
            await ws.send_str(
                json.dumps({"type": "event_propagate", "payload": fake_event_data})
            )

            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    payload = json.loads(msg.data)

                    if payload.get("type", None) == "close":
                        await ws.close()
                        return

                    if payload.get("verb", None) == "get":
                        payload = {
                            "payload": {
                                "status": "success",
                                "text": "test text",
                                "payload": {},
                            }
                        }

                    await ws.send_str(json.dumps({**payload, "type": "response"}))

                elif msg.type == aiohttp.WSMsgType.ERROR:
                    print("ws connection closed with exception %s" % ws.exception())

        app = web.Application()
        app.router.add_get("/ws", ws_handler)
        return app

    def tearDown(self):
        if hasattr(self, "api_client") and hasattr(self.api_client, "disconnect"):
            asyncio.get_event_loop().run_until_complete(self.api_client.disconnect())

    @patch("aiohttp.ClientSession")
    def test_get(self, session_client_factory_mock):
        async def interact():
            try:
                result = await self.api_client.get("/test")
                expected = ApiResponse({"status", "success"})
                self.assertEqual(expected.status, result.status)
                self.assertEqual(expected.text, result.text)
                self.assertEqual(expected.json, result.json)

            finally:
                await self.api_client.ws.send_json({"type": "close"})

        async def propagate(queue):
            pass

        async def async_test_get(loop, client):
            session_client_factory_mock.return_value = client

            self.api_client = ApiWebsocketClient(
                loop=loop,
                interact=interact,
                propagate=propagate,
                host="localhost",
                port=8080,
            )
            self.api_client.base_url = "/ws"
            await self.api_client.connect()

        run_aiohttp_client_test(self.get_application(), async_test_get)

    @patch("aiohttp.ClientSession")
    def test_post(self, session_client_factory_mock):
        async def interact():
            try:
                data = {"vern": "post", "status": "success", "text": "is a payload"}
                result = await self.api_client.post("/test", payload=data)
                expected = ApiResponse(data)
                self.assertEqual(expected.status, result.status)
                self.assertEqual(expected.text, result.text)
                self.assertEqual(expected.json, result.json)
            finally:
                await self.api_client.ws.send_json({"type": "close"})

        async def propagate(queue):
            pass

        async def async_test_post(loop, client):
            session_client_factory_mock.return_value = client

            self.api_client = ApiWebsocketClient(
                loop=loop,
                interact=interact,
                propagate=propagate,
                host="localhost",
                port=8080,
            )
            self.api_client.base_url = "/ws"
            await self.api_client.connect()

        run_aiohttp_client_test(self.get_application(), async_test_post)

    @patch("aiohttp.ClientSession")
    def test_put(self, session_client_factory_mock):
        async def interact():
            try:
                data = {"status": "success", "text": "is a payload"}
                result = await self.api_client.put("/test", payload=data)
                expected = ApiResponse(data)
                self.assertEqual(expected.status, result.status)
                self.assertEqual(expected.text, result.text)
                self.assertEqual(expected.json, result.json)
            finally:
                await self.api_client.ws.send_json({"type": "close"})

        async def propagate(queue):
            pass

        async def async_test_put(loop, client):
            session_client_factory_mock.return_value = client

            self.api_client = ApiWebsocketClient(
                loop=loop,
                interact=interact,
                propagate=propagate,
                host="localhost",
                port=8080,
            )
            self.api_client.base_url = "/ws"
            await self.api_client.connect()

        run_aiohttp_client_test(self.get_application(), async_test_put)

    @patch("aiohttp.ClientSession")
    def test_delete(self, session_client_factory_mock):
        async def interact():
            try:
                data = {"status": "success", "text": "is a payload"}
                result = await self.api_client.delete("/test", payload=data)
                expected = ApiResponse(data)
                self.assertEqual(expected.status, result.status)
                self.assertEqual(expected.text, result.text)
                self.assertEqual(expected.json, result.json)
            finally:
                await self.api_client.ws.send_json({"type": "close"})

        async def propagate(queue):
            pass

        async def async_test_delete(loop, client):
            session_client_factory_mock.return_value = client

            self.api_client = ApiWebsocketClient(
                loop=loop,
                interact=interact,
                propagate=propagate,
                host="localhost",
                port=8080,
            )
            self.api_client.base_url = "/ws"
            await self.api_client.connect()

        run_aiohttp_client_test(self.get_application(), async_test_delete)


class NetworkPropDummyEvent(Event):
    def __init__(self):
        super().__init__(
            priority=10, schema=Schema({"prop_1": int, "prop_2": And(str, len)})
        )
