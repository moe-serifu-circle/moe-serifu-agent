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

            _, event = await queue.get()
            print(event)

            if isinstance(event, events.AddScriptEvent):
                insert_new_script = ScriptEntity.insert().values(
                    name=event.data["name"],
                    crontab=event.data["crontab"],
                    script_contents=event.data["script_contents"]
                )


                conn = await self.database.connect()
                result = await conn.execute(ScriptEntity.filter(ScriptEntity.name == event.data["name"]).count())

                if result == 0:
                    await conn.execute(insert_new_script)
                else:
                    new_event = events.AddScriptFailedEvent()
                    new_event.init({
                            "error": f"Failed to add script {event.data['name']}." ,
                            "description": "A script with this name already exists.",
                            "description_verbose": "A script with this name already exists.Delete the script with this name then try again",
                    })
                    await supervisor.fire_event(
                        new_event
                    )
                    #await conn.close()
                

class TriggerScriptRunHandler(EventHandler):
    """
    Handles TriggerScriptRun Events
    """
    def __init__(self, loop, event_bus, database, logger, config=None):
        super().__init__(loop, event_bus, database, logger, config)

    async def handle(self):
        with self.event_bus.subscribe([events.TriggerScriptRunEvent]):
            pass

    
