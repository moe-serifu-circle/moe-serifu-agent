import asyncio

class Subscription:
    def __init__(self, event_bus, queue):
        self.event_bus = event_bus
        self.queue = queue

    def __enter__(self):
        return self.queue

    def __exit__(self):
        self.event_bus.unsubscribe(self.queue)

    

class EventBus:
    """The event bus is responsible for tracking event queues and pushing new events into the event queues so that the
    event handlers can wait until a new event is sent to them via their event queue."""

    def __init__(self, loop):
        """Creates a new event bus"""
        self.loop = loop
        self.event_queues = {}
        self.registered_event_types = {}
        self.propagation_hooks = []

        self.subscriptions = {}

        self.queues = []


    def subscribe(self, event_types):
        new_queue = asyncio.PriorityQueue(loop=self.loop)

        for event_type in event_types:
            if event_type not self.subscriptions:
                self.subscriptions[event_type] = set(new_queue)
            else:
                self.subscriptions[event_type].add(new_queue)

        return Subscription(self, new_queue)

    def unsubscribe(self, queue):
        for _,subscriptions in self.subscriptions:
            if queue in subscriptions:
                subscriptions.remove(queue)

    async def fire_event(self, new_event):
        """Fires an event to each event handler via its corresponding event queue.

        Parameters
        ----------
        new_event : msa.core.event.Event
            A subclass of msa.core.event.Event to propagate to event handlers."""
        for queue in self.queues:
            queue.put_nowait((new_event.priority, new_event))





