import json

class EventPropagationRouter:

    def __init__(self):
        self.app = None
        self.event_bus = None

    def register_propagate_subscription(self, event_bus):
        self.event_bus = event_bus
        event_bus.subscribe(".*", self.handle_event_propagate)

    async def app_start(self, app):
        self.app = app

    async def app_stop(self, app):
        pass
        #self.event_bus.unsubscri

    async def handle_event_propagate(self, event):
        if not event._network_propagate:
            return

        if "websockets" not in self.app:
            return

        for ws in self.app['websockets']:
            try:
                await ws.send_str(json.dumps({
                    "type": "event_propagate",
                    "payload": event.get_metadata()
                }))
            except:
                pass
