import asyncio
import sys

from msa import supervisor
from msa.coroutine import Coroutine, reschedule

from msa.plugins.terminal_input.event import TextInputEvent

class ConversationCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

    @reschedule
    async def work(self, event_queue):
        print("What would you like to eat?\n1)Apples\n2)Pears")
        event = await event_queue.get()

        if not isinstance(event, TextInputEvent) or not event.propogate:
            await asyncio.sleep(0.1)
            return

        msg = event.data["text"]

        msg = msg.lower()

        if msg == "pears":
            print("Yummy, yummy pears!")

        elif msg == "apples":
            print("Keeping the doctor away and all that :D")

        elif msg == "quit":
            print("Well if you insist... Bye, bye!", flush=True)
            from msa.supervisor import stop
            stop()
            return

        else:
            print(f"Well I don't know what {msg} is but it sounds like you enjoy it!")

        print() # add an extra line for visibility

        await asyncio.sleep(0.5)
