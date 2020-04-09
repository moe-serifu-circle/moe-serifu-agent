import aiohttp
from aiohttp import web
import json
from uuid import uuid4

from msa.api import ApiContext
from msa.server.server_request import SeverRequest


class RouteAdapter:
    cache = None

    def __init__(self):
        self.routes = {"get" : {}, "put": {}, "post": {}, "delete": {}}
        self.sync_routes = { "get": {}, "put": {}, "post": {}, "delete":{}}
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

    def generate_websocket_route(self):
        route_adapter = self
        async def websocket_route(response):

            peername = response.transport.get_extra_info('peername')
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
                json.dumps({
                    "type": "notify_id",
                    "payload": {
                        "id": client_id
                    }
                })
            )

            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    payload = json.loads(msg.data)

                    if "verb" not in payload:
                        await ws.send_str(
                            json.dumps({"type": "error", "message": "Websocket payload requires a verb field."}))
                        continue

                    if "route" not in payload:
                        await ws.send_str(
                            json.dumps({"type": "error", "message": "Websocket payload requires a route field."}))
                        continue

                    if "data" not in payload:
                        await ws.send_str(
                            json.dumps({"type": "error", "message": "Websocket payload requires a data field."}))

                    request = SeverRequest(
                        client_id,
                        payload["verb"],
                        payload["route"],
                        payload["data"],
                    )

                    try:
                        route_func = route_adapter.lookup_route(request.verb, request.route)
                    except Exception as e:
                        await ws.send_str(json.dumps({"type": "error", "message": str(e)}))
                        continue

                    try:
                        response = await route_func(ApiContext.websocket, request)

                        if response is None:
                            wrapped_response = {"type": "empty_response"}
                        else:
                            wrapped_response = {"type": "response", "payload": response}
                        await ws.send_str(json.dumps(wrapped_response))
                    except Exception as e:
                        await ws.send_str(json.dumps({
                            "type": "error",
                            "payload": {
                                "message": str(e)
                            }
                        }))
                        continue

                elif msg.type == aiohttp.WSMsgType.ERROR:
                    print('ws connection closed with exception %s' %
                          ws.exception())

                    self.app["websockets"].remove(ws)

            print(f"Client {host}:{port} disconnected")
            return ws
        return websocket_route

    def get_route_table(self):
        all_routes = [ ]

        def route_wrapper(func):
            async def wrapped_route(request, raw_data=None):
                if raw_data:
                    payload = raw_data
                else:
                    dump = await request.read()
                    decoded = dump.decode("utf-8")
                    if len(decoded) != 0:
                        payload = json.loads(decoded)
                    else: 
                        payload = None

                response = await func(ApiContext.rest, payload)
                return web.Response(**response)
            return wrapped_route

        for route, route_func in self.routes["get"].items():
            all_routes.append(
                web.get(
                    route,
                    route_wrapper(route_func)
                ))

        for route, route_func in self.routes["put"].items():
            all_routes.append(
                web.put(
                    route,
                    route_wrapper(route_func)
                ))

        for route, route_func in self.routes["post"].items():
            all_routes.append(
                web.post(
                    route, 
                    route_wrapper(route_func)
                ))

        for route, route_func in self.routes["delete"].items():
            all_routes.append(
                web.delete(
                    route,
                    route_wrapper(route_func)
                ))

        all_routes.append(web.get("/ws", self.generate_websocket_route()))
        return all_routes

    def lookup_route(self, verb, route):
        if verb in self.routes:
            if route in self.routes[verb]:
                return self.routes[verb][route]
        return None

