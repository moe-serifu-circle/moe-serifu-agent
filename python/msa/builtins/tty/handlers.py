from prompt_toolkit.eventloop import use_asyncio_event_loop
from prompt_toolkit import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout

from msa.core.event_handler import EventHandler
from msa.builtins.tty.events import *

from msa.core import supervisor

use_asyncio_event_loop(supervisor.loop)

class TtyInputHandler(EventHandler):
    """Listens to stdin for terminal input and then fires a TextInputEvent."""

    def __init__(self, loop, event_queue):
        super().__init__(loop, event_queue)

        self.prompt_session = PromptSession()

    async def handle(self):

        with patch_stdout():
            msg = await self.prompt_session.prompt("$> ", async_=True)

        event = TextInputEvent()
        event.init({"message": msg})

        supervisor.fire_event(event)
