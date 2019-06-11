import asyncio
import aiocron

from msa.core.event_handler import EventHandler
from msa.core import supervisor

from msa.builtins.scripting import events
from msa.builtins.scripting.entities import *


class ScriptManager:
    def __init__(self):
        pass



class AddScriptHandler(EventHandler):
    """
    Handles AddScript Events
    """

    def __init__(self, loop, event_bus, database, logger, config=None):
        super().__init__(loop, event_bus, database, logger, config)

    async def handle(self):

        with self.event_bus.subscribe([events.AddScriptEvent]) as queue:
            event = await queue.get()

            if isinstance(event, events.AddScriptEvent):
                pass




