import asyncio


class EventBus:
    """The event bus is responsible for tracking event queues and pushing new events into the event queues so that the
    event handlers can wait until a new event is sent to them via their event queue."""

    def __init__(self, loop):
        """Creates a new event bus"""
        self.loop = loop
        self.event_queues = {}
        self.registered_event_types = {}
        self.propagation_hooks = []

        self.queues = []

    def create_event_queue(self):
        """Creates a new event queue. Each handler should receive its own event queue."""

        new_queue = asyncio.PriorityQueue(loop=self.loop)
        self.queues.append(new_queue)

        return new_queue

    async def fire_event(self, new_event):
        """Fires an event to each event handler via its corresponding event queue."""
        for queue in self.queues:
            queue.put_nowait((new_event.priority, new_event))





