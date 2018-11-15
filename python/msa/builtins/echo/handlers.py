import asyncio

from msa.core import supervisor
from msa.core.event_handler import EventHandler

from msa.builtins.command_registry.events import RegisterCommandEvent
from msa.builtins.echo.events import EchoCommandEvent


class EchoHandler(EventHandler):
    """Checks for EchoCommnadEvents and displays the text provided in them"""

    async def init(self):

        data = {"event_constructor": EchoCommandEvent,
                "invoke": "echo",
                "describe": "Echos provided text back through the terminal",
                "usage": "'echo [text]'"}

        register_event = RegisterCommandEvent()
        register_event.init(data)

        supervisor.fire_event(register_event)

    async def handle(self):

        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, EchoCommandEvent):
            await self.echo_command(event)

        await asyncio.sleep(0.1)

    async def echo_command(self, event):
        """Displays th text provided in the EchoCommandEvent"""

        text = event.data["raw_text"][5:]

        self.print(text)  # TODO refactor to use TTY output event

    def print(self, text):
        """temportary work around to allow unit testing, should instead create TTy out event"""
        print(text)
