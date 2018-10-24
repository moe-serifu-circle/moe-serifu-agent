import asyncio
from prompt_toolkit.eventloop import use_asyncio_event_loop
from prompt_toolkit import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout

from msa import supervisor
from msa.coroutine import Coroutine, reschedule
from msa.prompt import Prompt

from msa.builtins.terminal_input.event import TextInputEvent

use_asyncio_event_loop()

class KeyboardInputCoroutine(Coroutine):

    def __init__(self):
        super().__init__()
        self.prompt = Prompt()
        self.printed_prompt = False

        self.prompt_session = PromptSession()


    @reschedule
    async def work(self, event_queue):
        with patch_stdout():
            msg = await self.prompt_session.prompt("$> ", async_=True)

        new_event = TextInputEvent()
        new_event.init(msg)
        await supervisor.propogate_event(new_event)

        await asyncio.sleep(0.1)

