import asyncio

from msa.core import supervisor
from msa.core.event_handler import EventHandler

from msa.builtins.command_registry.events import RegisterCommandEvent
from msa.builtins.command.events import QuitCommandEvent


class QuitHandler(EventHandler):
    """Checks for QuitCommnadEvents tells the supervisor to quit upon finding one"""

    async def init(self):

        data = {"event_constructor": QuitCommandEvent,
                "invoke": "quit",
                "describe": "Shuts down the current Moe Serifu Agent instance",
                "usage": "'quit'"}

        register_event = RegisterCommandEvent()
        register_event.init(data)

        supervisor.fire_event(register_event)

    async def handle(self):

        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, QuitCommandEvent):
            from msa.core import supervisor
            print("Ok, shutting down...")  # Do not refactor to PrintEvent as it will be ignored after stop() is called
            supervisor.stop()

        await asyncio.sleep(0.1)

