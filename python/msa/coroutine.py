import sys
import asyncio

from msa.prompt import Prompt


class Coroutine:
    def __init__(self):
        self.loop = asyncio.get_event_loop()

    async def work(self, *args):
        raise NotImplementedError



def reschedule(fn, args=[]):
    async def wrapped_cb(self, args=[]):

        from msa import supervisor
        if not supervisor.stop_loop:

            result = await fn(self, *args)
            loop = asyncio.get_event_loop()
            asyncio.ensure_future(self.work(), loop=loop)

            return result

    return wrapped_cb






class HelloWorldCoroutine(Coroutine):

    @reschedule
    async def work(self, *args):
        print("hello world")
        sys.stdout.flush()

        await asyncio.sleep(3)
