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
            self.logger.debug("First time printing prompt, not inserting delay")
            self.first = False
        else:
            # insert a delay to allow events to print their text
            # before we print the prompt
            self.logger.debug("Inserting delay, before printing prompt to allow events to print their text.")
            await asyncio.sleep(0.5)

        if supervisor.should_stop():
            self.logger.debug("Supervisor has shut down, aborting rest of handler logic.")
            return # quit because the supervisor may have begun shutting down while we were waiting.

        self.logger.debug("Printing prompt")
        print(">> ", end="", flush=True)
        sys.stdout.flush()

        self.logger.debug("Listening for input")
        msg = await self.prompt.listen(wait=True)
        msg = msg.rstrip()
        self.logger.debug("Captured input: \"{}\"".format(msg))


        if msg is None or not len(msg):
            self.logger.debug("Not creating event because message was empty")
            return

        self.logger.debug("Creating new TextInputEvent.")
        event = TextInputEvent()
        event.init({"message": msg})

        self.logger.debug("Firing new TextInputEvent.")
        supervisor.fire_event(event)


class TtyOutputHandler(EventHandler):
    """Listens to for TextOutputEvents and StyledTextOutputEvents and prints text to TTY."""

    async def handle(self):
        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, TextOutputEvent):
            self.logger.debug("Handling TextOutputEvent: \"{}\"".format(event.data["message"]))
            self.print(event.data["message"])

        elif isinstance(event, StyledTextOutputEvent):
            self.logger.debug("Handling StyledTextOutputEvent: \"{}\"".format(event.data["message"]))
            self.print(_style_text(event.data["message"]))

    def print(self, *args, **kwargs):
        """A wrapper around print. Helps with unit tests."""
        colorama.init()
        print(*args, **kwargs, end="")




