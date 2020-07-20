from collections import deque
import datetime
from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.signals import events


class StartupEventTrigger(EventHandler):
    """
    Fires off a startup event and then exits
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)

    async def init(self):
        # trigger startup hook later
        self.loop.call_later(1, self.trigger_event)

    def trigger_event(self):
        new_event = events.StartupEvent()
        new_event.init(
            {"timestamp": datetime.datetime.now().strftime("%Y-%m-%d, %H:%M:%S:%f")}
        )
        supervisor.fire_event(new_event)


class NetworkPropagateEventHandler(EventHandler):
    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)

        event_bus.subscribe(".*", self.handle)

        self.buffered_events = deque(maxlen=10)

    async def handle(self, event):

        if isinstance(event, events.RequestDisburseEventsToNetworkEvent):
            self.handle_disburse_request()
            return

        if not event._network_propagate:
            return

        self.buffered_events.append(event)

    def handle_disburse_request(self):
        new_event = events.DisburseEventsToNetworkEvent().init(
            {"events": [event.get_metadata() for event in self.buffered_events]}
        )
        self.buffered_events.clear()
        supervisor.fire_event(new_event)
