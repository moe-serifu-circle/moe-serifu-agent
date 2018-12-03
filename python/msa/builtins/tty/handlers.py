import sys
import asyncio

import colorama

from msa.core.event_handler import EventHandler
from msa.builtins.tty.events import *
from msa.builtins.tty.style import _style_text
from msa.builtins.tty.prompt import Prompt

from msa.core import supervisor

class TtyInputHandler(EventHandler):
    """Listens to stdin for terminal input and then fires a TextInputEvent."""

    def __init__(self, loop, event_queue, logger, config=None):
        super().__init__(loop, event_queue, logger, config)

        self.prompt = Prompt(loop=loop)

        self.first = True

    async def handle(self):

        if self.first:
            self.first = False
        else:
            # insert a delay to allow events to print their text
            # before we print the prompt
            await asyncio.sleep(0.5)

        if supervisor.should_stop():
            return # quit because the supervisor may have begun shutting down while we were waiting.

        print(">> ", end="", flush=True)
        sys.stdout.flush()
        msg = await self.prompt.listen(wait=True)
        msg = msg.rstrip()



        if msg is None or not len(msg):
            return

        event = TextInputEvent()
        event.init({"message": msg})

        supervisor.fire_event(event)


class TtyOutputHandler(EventHandler):

    async def handle(self):
        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, TextOutputEvent):
            self.print(event.data["message"])

        elif isinstance(event, StyledTextOutputEvent):
            self.print(_style_text(event.data["message"]))

    def print(self, *args, **kwargs):
        """A wrapper around print. Helps with unit tests."""
        colorama.init()
        print(*args, **kwargs, end="")




