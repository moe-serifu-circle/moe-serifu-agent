import asyncio


class EventBus:

    def __init__(self):
        self.event_queues = {}
        self.registered_event_types = {}
        self.propagation_hooks = []

        self.queues = []

    def create_event_queue(self):

        new_queue = asyncio.PriorityQueue()
        self.queues.append(new_queue)

        return new_queue

    async def fire_event(self, new_event):
        for queue in self.queues:
            await queue.put_nowait((new_event.priority, new_event))





