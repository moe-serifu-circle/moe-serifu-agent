import asyncio

class EventHandler:

    def __init__(self, loop: asyncio.AbstractEventLoop, event_queue: asyncio.Queue):
        self.loop = loop
        self.event_queue = event_queue

    async def init(self):
        pass

    async def handle_wrapper(self):

        from msa.core import supervisor

        while not supervisor.should_stop():
            await self.handle()
            await asyncio.sleep(0.01)


    async def handle(self):
        raise NotImplementedError()




