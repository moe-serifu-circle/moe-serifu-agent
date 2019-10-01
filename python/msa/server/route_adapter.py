import aiohttp
from aiohttp import web
import json

class RouteAdapter:
    def __init__(self):
        self.routes = {"get" : {}, "put": {}, "post": {}, "delete": {}}
        self.sync_routes = { "get": {}, "put": {}, "post": {}, "delete":{}}

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
                host, port = peername
            else:
                host, port = "Unknown", "xxxx"

            ws = web.WebSocketResponse()
            await ws.prepare(response)

            print(f"Client {host}:{port} connected")

            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    payload = json.loads(msg.data)

                    if "verb" not in payload:
                        await ws.send_str(json.dumps({"type": "error", "message": "Websocket payload requires a verb field."}))
                        continue

                    if "route" not in payload:
                        await ws.send_str(json.dumps({"type": "error", "message": "Websocket payload requires a route field."}))
                        continue

                    if "data" not in payload:
                        await ws.send_str(json.dumps({"type": "error", "message": "Websocket payload requires a data field."}))

                    try:
                        route_func = route_adapter.lookup_route(payload["verb"], payload["route"])
                    except Exception as e:
                        await ws.send_str(json.dumps({"type": "error", "message": str(e)}))
                        continue

                    try:
                        request_data = payload["data"]
                        response = await route_func(request=None, raw_data=request_data)
                        response["type"] = "response"
                        await ws.send_str(json.dumps(response))
                    except Exception as e:
                        await ws.send_str(json.dumps({"type": "error", "message": str(e)}))
                        continue

                elif msg.type == aiohttp.WSMsgType.ERROR:
                    print('ws connection closed with exception %s' %
                          ws.exception())
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

                response = await func(payload)
                if "json" in response:
                    j = response["json"]
                    del response["json"]
                    response["data"] = json.dumps(j)
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

