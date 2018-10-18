import sys
import asyncio

from prompt import Prompt


class Coroutine:
    def __init__(self):
        self.loop = asyncio.get_event_loop()

    async def work(self, *args):
        raise NotImplementedError



def reschedule(fn, args=[]):
    async def wrapped_cb(self, args=[]):
        import supervisor
        if not supervisor.stop_loop:

            result = await fn(self, *args)
            loop = asyncio.get_event_loop()
            asyncio.ensure_future(self.work(), loop=loop)

            return result

    return wrapped_cb




class KeyboardInputCoroutine(Coroutine):

    def __init__(self):
        super().__init__()
        self.prompt = Prompt()

    @reschedule
    async def work(self, *args):
        msg = await self.prompt("What would you like to eat?:\n1)Pears\n2)Apples\n> ", end="", wait=True)

        msg = msg.lower()

        if msg == "pears":
            print("Yummy, yummy pears!")

        elif msg == "apples":
            print("Keeping the doctor away and all that :D")

        elif msg == "quit":
            print("Well if you insist... Bye, bye!")
            from supervisor import stop
            stop()
            return

        else:
            print(f"Well I don't know what {msg} is but it sounds like you enjoy it!")

        await asyncio.sleep(0.5)



class HelloWorldCoroutine(Coroutine):

    @reschedule
    async def work(self, *args):
        print("hello world")
        sys.stdout.flush()

        await asyncio.sleep(3)
