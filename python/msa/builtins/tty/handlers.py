import sys

from prompt_toolkit.eventloop import use_asyncio_event_loop
from prompt_toolkit.shortcuts import PromptSession
from prompt_toolkit.patch_stdout import patch_stdout

from msa.core.event_handler import EventHandler
from msa.builtins.tty.events import *
from msa.builtins.tty.style import _style_text

from msa.core import supervisor

use_asyncio_event_loop(supervisor.loop)



class TtyInputHandler(EventHandler):
    """Listens to stdin for terminal input and then fires a TextInputEvent."""

    def __init__(self, loop, event_queue):
        super().__init__(loop, event_queue)

        self.prompt_session = PromptSession()

    async def handle(self):

        with patch_stdout():
            msg = await self.prompt_session.prompt(">> ", async_=True)

        event = TextInputEvent()
        event.init({"message": msg})

        supervisor.fire_event(event)


class TtyOutputHandler(EventHandler):

    async def handle(self):

        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, TextOutputEvent):
            self.print(event.data["message"] + "\n")

        elif isinstance(event, StyledTextOutputEvent):
            self.print(_style_text(event.data["message"]) + "\n")

    def print(self, *args, **kwargs):
        """A wrapper around print. Helps with unit tests."""
        print(*args, **kwargs)


