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

        await supervisor.propogate_event(TextInputEvent(msg))

        await asyncio.sleep(0.1)

