import sys
import asyncio

from msa.prompt import Prompt


class Coroutine:
    def __init__(self):
        self.loop = asyncio.get_event_loop()

    async def work(self, event):
        raise NotImplementedError



def reschedule(fn):
    async def wrapped_cb(self, event_queue):

        from msa import supervisor
        if not supervisor.stop_loop:

            await fn(self, event_queue)

            loop = asyncio.get_event_loop()
            asyncio.ensure_future(self.work(event_queue), loop=loop)

    return wrapped_cb



class HelloWorldCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

        self.counter = 0

    @reschedule
    async def work(self, event):

        self.counter += 1
        if self.counter >= 30:
            print("hello world")
            self.counter = 0

        await asyncio.sleep(0.1)
