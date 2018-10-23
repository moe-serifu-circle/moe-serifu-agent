import asyncio

from msa import supervisor
from msa.coroutine import Coroutine, reschedule
from msa.prompt import Prompt

from msa.builtins.terminal_input.event import TextInputEvent

class KeyboardInputCoroutine(Coroutine):

    def __init__(self):
        super().__init__()
        self.prompt = Prompt()
        self.printed_prompt = False


    @reschedule
    async def work(self, event_queue):
        msg = await self.prompt("", end="", wait=True)

        new_event = TextInputEvent()
        new_event.init(msg)
        await supervisor.propogate_event(new_event)

        await asyncio.sleep(0.1)

