import asyncio
import re

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
        self.complex_subscriptions = {}
        self.result_listeners = {}
        self.event_result = {}

        self.queue = asyncio.PriorityQueue(loop=self.loop)

        self.task = None



    def subscribe(self, event_type, callback):

        if isinstance(event_type, str):
            if event_type not in self.complex_subscriptions:
                self.complex_subscriptions[event_type] = {callback}
            else:
                self.complex_subscriptions[event_type].add(callback)
        else:
            if event_type not in self.subscriptions:
                self.subscriptions[event_type] = {callback}
            else:
                self.subscriptions[event_type].add(callback)



    def unsubscribe(self, event_type, callback):
        if event_type in self.subscriptions:
            if callback in self.subscriptions[event_type]:
                self.subscriptions[event_type].remove(callback)

        if event_type in self.complex_subscriptions:
            if callback in self.complex_subscriptions[event_type]:
                self.complex_subscriptions[event_type].remove(callback)

    def _get_subscribers(self, event_type):
        subs = []

        if event_type in self.subscriptions.keys():
            subs.extend(self.subscriptions[event_type])

        for type_, listeners in self.complex_subscriptions.items():
            if re.match(type_, event_type.__name__):
                subs.extend(listeners)

        return list(set(subs))


    async def listen(self):
        """Listens for a new event to be passed into the event bus queue via EventBus.fire_event. """

        while True:

            _, event = await self.queue.get()

            event_type = type(event)
            subs = self._get_subscribers(event_type)
            if len(subs) == 0:
                print(f"WARNING: propagated event type \"{event_type}\" that nothing was subscribed to. Dropping event.")


            self.task = asyncio.gather(*[
                callback(event) 
                for callback in 
                    subs
                 ])

            result = await self.task

            if event_type in self.result_listeners:
                async_event = self.result_listeners[event_type]
                del self.result_listeners[event_type]
                self.event_result[event_type] = event
                async_event.set()


    async def listen_for_result(self, event_type):
        if event_type not in self.result_listeners:
            event = asyncio.Event()
            self.result_listeners[event_type] = event
        else:
            event = self.result_listeners[event_type]

        await event.wait()
        return self.event_result[event_type]



    async def fire_event(self, new_event):
        """Fires an event to each event handler via its corresponding event queue.

        Parameters
        ----------
        new_event : msa.core.event.Event
            A subclass of msa.core.event.Event to propagate to event handlers."""
        # get queues for event type
        event_type = type(new_event)

        subs = self._get_subscribers(event_type)
        if len(subs) == 0:
            print(f"WARNING: attempted to propagate event type \"{event_type}\" that nothing was subscribed to. Dropping event.")
        else:
            self.queue.put_nowait((new_event.priority, new_event))





