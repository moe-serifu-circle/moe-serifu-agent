import aiohttp
from aiohttp import web
import json
from uuid import uuid4
import traceback

from msa.server.server_request import SeverRequest
from msa.server.server_response import (
    ServerResponse,
    ServerResponseText,
    ServerResponseType,
    ServerResponseJson,
)
from msa.server.url_param_parser import UrlParamParser


class RouteAdapter:
    cache = None

    def __init__(self):
        self.routes = {"get": {}, "put": {}, "post": {}, "delete": {}}
        self.param_matchers = []
        self.app = None

    @staticmethod
    def _get_instance():
        if RouteAdapter.cache is None:
            instance = RouteAdapter()
            RouteAdapter.cache = instance
            return instance
        else:
            return RouteAdapter.cache

    def _register_route(self, verb, route):
        if route in self.routes[verb]:
            raise Exception("Route GET {} already registered".format(route))

        def add_route(func):
            self.routes[verb][route] = func

        return add_route

    def get(self, route):
        return self._register_route("get", route)

    def put(self, route):
        return self._register_route("put", route)

    def post(self, route):
        return self._register_route("post", route)

    def delete(self, route):
        return self._register_route("delete", route)

    def register_app(self, app):
        self.app = app
        self.app["websockets"] = []

    def lookup_route(self, verb, route):
        if verb not in self.routes:
            raise Exception("Invalid route verb: {}".format(verb))
        if route not in self.routes[verb]:
            raise Exception("Invalid route : {}".format(route))

        return self.routes[verb][route]

    def _build_generic_response(self, response):
        if not isinstance(response, ServerResponse) or not issubclass(
            type(response), ServerResponse
        ):
            raise Exception(
                "Invalid server response type. Server response, must be a subclass of "
                "ServerResponse."
            )
        return response.get_data()

    def generate_websocket_route(self):
        route_adapter = self

        async def websocket_route(response):

            peername = response.transport.get_extra_info("peername")
            if peername is not None:
                host, port = peername[:2]
            else:
                host, port = "Unknown", "xxxx"
            uuid = str(uuid4())
            client_id = f"{host}:{port}:{uuid}"

            ws = web.WebSocketResponse()
            await ws.prepare(response)

            print(f"Client {host}:{port} connected")
            self.app["websockets"].append(ws)

            # send client id to client
            await ws.send_str(
                json.dumps({"type": "notify_id", "payload": {"id": client_id}})
            )

            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    payload = json.loads(msg.data)

                    if "verb" not in payload:
                        await ws.send_str(
                            json.dumps(
                                {
                                    "type": "error",
                                    "message": "Websocket payload requires a verb field.",
                                }
                            )
                        )
                        continue

                    if "route" not in payload:
                        await ws.send_str(
                            json.dumps(
                                {
                                    "type": "error",
                                    "message": "Websocket payload requires a route field.",
                                }
                            )
                        )
                        continue

                    if "payload" not in payload:
                        await ws.send_str(
                            json.dumps(
                                {
                                    "type": "error",
                                    "message": "Websocket payload requires a payload field.",
                                }
                            )
                        )

                    try:
                        (
                            route_func,
                            url_params,
                        ) = route_adapter.lookup_route_and_resolve_url_params(
                            payload["verb"], payload["route"]
                        )
                    except Exception as e:
                        await ws.send_str(
                            json.dumps({"type": "error", "message": str(e)})
                        )
                        continue

                    request = SeverRequest(
                        client_id,
                        payload["verb"],
                        payload["route"],
                        payload["payload"],
                        url_params,
                    )

                    try:
                        response = await route_func(request)

                        wrapped_response = {
                            "type": "response",
                            "payload": self._build_generic_response(response),
                        }

                        await ws.send_str(json.dumps(wrapped_response))
                    except Exception as e:
                        await ws.send_str(
                            json.dumps(
                                {"type": "error", "message": traceback.format_exc()}
                            )
                        )
                        continue

                elif msg.type == aiohttp.WSMsgType.ERROR:
                    print("ws connection closed with exception %s" % ws.exception())

                    self.app["websockets"].remove(ws)

            print(f"Client {host}:{port} disconnected")
            return ws

        return websocket_route

    def get_route_table(self):
        all_routes = []

        def route_wrapper(verb, route, func):

            self.param_matchers.append(UrlParamParser(route))

            async def wrapped_route(request, raw_data=None):
                if raw_data:
                    payload = raw_data
                    url_vars = (
                        {}
                    )  # TODO: find some way to get url params from local client
                else:
                    dump = await request.read()
                    url_vars = request.match_info
                    decoded = dump.decode("utf-8")
                    if len(decoded) != 0:
                        payload = json.loads(decoded)
                    else:
                        payload = None

                server_request = SeverRequest(
                    "rest" + str(uuid4()), verb, route, payload, url_vars
                )

                response = await func(server_request)
                return web.json_response(self._build_generic_response(response))

            return wrapped_route

        for route, route_func in self.routes["get"].items():
            all_routes.append(web.get(route, route_wrapper("get", route, route_func)))

        for route, route_func in self.routes["put"].items():
            all_routes.append(web.put(route, route_wrapper("put", route, route_func)))

        for route, route_func in self.routes["post"].items():
            all_routes.append(web.post(route, route_wrapper("post", route, route_func)))

        for route, route_func in self.routes["delete"].items():
            all_routes.append(
                web.delete(route, route_wrapper("delete", route, route_func))
            )

        all_routes.append(web.get("/ws", self.generate_websocket_route()))
        return all_routes

    def lookup_route(self, verb, route):
        if verb in self.routes:
            if route in self.routes[verb]:
                return self.routes[verb][route]
        return None

    def lookup_route_and_resolve_url_params(self, verb, route):
        if verb in self.routes:
            for matcher in self.param_matchers:
                if matcher.match(route) and matcher.route in self.routes[verb]:
                    return (
                        self.routes[verb][matcher.route],
                        matcher.resolve_params(route),
                    )

        raise Exception("Failed to resolve url:", verb, route)
