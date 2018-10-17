import sys
import asyncio

from wrapper import reschedule
from prompt import Prompt


class Coroutine:
    async def work(self, *args):
        raise NotImplementedError


def reschedule(fn, args=[]):
    async def wrapped_cb(self, args=[]):

        result = await fn(self, *args)
        loop = asyncio.get_event_loop()
        asyncio.ensure_future(self.work(), loop=loop)

        return result

    return wrapped_cb




class KeyboardInputCoroutine(Coroutine):

    def __init__(self):
        self.prompt = Prompt()

    @reschedule
    async def work(self, *args):
        msg = await self.prompt("prompt: ", wait=True)
        print(msg)

        await asyncio.sleep(0.5)



class HelloWorldCoroutine(Coroutine):

    @reschedule
    async def work(self, *args):
        print("hello world")
        sys.stdout.flush()

        await asyncio.sleep(3)
