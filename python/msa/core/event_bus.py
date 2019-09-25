import asyncio

class Subscription:
    def __init__(self, event_bus, queue):
        self.event_bus = event_bus
        self.queue = queue

    def __enter__(self):
        return self.queue

    def __exit__(self, type, value, traceback):
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
        self.queue = asyncio.PriorityQueue(loop=self.loop)

        self.task = None



    def subscribe(self, event_type, callback):

        if event_type not in self.subscriptions:
            self.subscriptions[event_type] = {callback}
        else:
            self.subscriptions[event_type].add(callback)


    def unsubscribe(self, event_type, callback):
        if event_type in self.subscriptions:
            if callback in self.subscriptions[event_type]:
                self.subscriptions[event_type].remove(callback)

    async def listen(self):
        """Listens for a new event to be passed into the event bus queue via EventBus.fire_event. """

        _, event = await self.queue.get()

        event_type = type(event)
        if event_type not in self.subscriptions.keys():
            print(f"WARNING: propagated event type \"{event_type}\" that nothing was subscribed to. Dropping event.")

        self.task = asyncio.gather(*[
            callback(event) 
            for callback in 
            self.subscriptions[event_type]
                 ])

        await self.task



    async def fire_event(self, new_event):
        """Fires an event to each event handler via its corresponding event queue.

        Parameters
        ----------
        new_event : msa.core.event.Event
            A subclass of msa.core.event.Event to propagate to event handlers."""
        # get queues for event type
        event_type = type(new_event)

        if event_type not in self.subscriptions.keys():
            print(f"WARNING: attempted to propagate event type \"{event_type}\" that nothing was subscribed to. Dropping event.")
        else:
            self.queue.put_nowait((new_event.priority, new_event))





