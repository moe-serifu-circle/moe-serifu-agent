import aiohttp
from aiohttp import web
import json

class RouteAdapter:
    def __init__(self):
        self.routes = {"get" : {}, "put": {}, "post": {}, "delete": {}}


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
            ws = web.WebSocketResponse()
            await ws.prepare(response)

            async for msg in ws:
                if msg.type == aiohttp.WSMessageType.TEXT:
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
                        await route_func(request=None, raw_data=request_data)
                    except Exception as e:
                        await ws.send_str(json.dumps({"type": "error", "message": str(e)}))
                        continue
                    

        return websocket_route

    def get_route_table(self):
        all_routes = [ ]

        for route, route_func in self.routes["get"].items():
            all_routes.append(web.get(route, route_func))

        for route, route_func in self.routes["put"].items():
            all_routes.append(web.put(route, route_func))

        for route, route_func in self.routes["post"].items():
            all_routes.append(web.post(route, route_func))

        for route, route_func in self.routes["delete"].items():
            all_routes.append(web.delete(route, route_func))

        all_routes.append(web.get("/ws", self.generate_websocket_route()))
        return all_routes

