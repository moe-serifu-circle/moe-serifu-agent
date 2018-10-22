import asyncio

from msa.coroutine import Coroutine, reschedule

from msa.builtins.print.event import PrintTextEvent

class PrintCoroutine(Coroutine):

    def __init__(self):
        super().__init__()


    @reschedule
    async def work(self, event_queue):
        event = await event_queue.get()

        if isinstance(event, PrintTextEvent):
            print(event.data["text"])

        await asyncio.sleep(0.1)

