from functools import partial
import asyncio
import aiocron

from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.scripting import events
from msa.builtins.scripting.entities import ScriptEntity
from msa.api import MsaLocalApiWrapper
from sqlalchemy.sql import select


class ScriptManager:
    __shared_state = None
    def __init__(self, loop=None, database=None):

        if ScriptManager.__shared_state is None:
            ScriptManager.__shared_state = {}
            self.__dict__ = ScriptManager.__shared_state

            self.loop = loop
            self.running_scripts = {}

            self.local_api = MsaLocalApiWrapper(database).get_api()

            self.globals = {
                "msa_api":  self.local_api
            }
        else:
            self.__dict__ = ScriptManager.__shared_state



    async def run_script(self, name, script_content, crontab_definition=None):
        if crontab_definition is not None:
            while True:
                await aiocron.crontab(crontab_definition).next()
        else:
            # run once and exit
            exec(script_content.strip(), self.globals, {})

        self.script_finished(name)

    def schedule_script(self, name, script_content, crontab_definition=None):
        if name not in self.running_scripts:

            script_coro = partial(self.run_script, name, script_content, crontab_definition)
            self.running_scripts[name] = self.loop.create_task(script_coro)

    def script_finished(self, name):
        del self.running_scripts[name]

    def shutdown(self):
        # schedule shutdown
        self.loop.create_task(self.async_shutdown())
        
    async def async_shutdown(self):
        for name, task in self.running_scripts.items():
            # todo: log the name of the script being shut down
            task.cancel()

            try:
                await task
            except asyncio.CancelledError:
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

            if isinstance(event, events.AddScriptEvent):


                with self.database.connect() as conn:
                    result = await conn.execute(ScriptEntity.filter(ScriptEntity.name == event.data["name"]).count())

                    if result == 0:
                        insert_new_script = ScriptEntity.insert().values(
                            name=event.data["name"],
                            crontab=event.data["crontab"],
                            script_contents=event.data["script_contents"]
                        )
                        await conn.execute(insert_new_script)

                        self.loop.create_task()
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
                

class TriggerScriptRunHandler(EventHandler):
    """
    Handles TriggerScriptRun Events
    """
    def __init__(self, loop, event_bus, database, logger, config=None):
        super().__init__(loop, event_bus, database, logger, config)
        self.script_manager = ScriptManager(loop, database)

        self.started = False

    async def async_init(self):
        # load all scripts and crontabs
        with self.database.connect() as conn:
            result = await conn.execute(ScriptEntity.select())

            # TODO: load specific script and use script manager to run immediately

        self.started = True

    async def handle(self):
        if not self.started:
            await self.async_init()

        with self.event_bus.subscribe([events.TriggerScriptRunEvent]):
            pass

    
class StartupEventHandler(EventHandler):
    """
    Handles Startup Event to start the script manager
    """
    def __init__(self, loop, event_bus, database, logger, config=None):
        super().__init__(loop, event_bus, database, logger, config)
        self.script_manager = ScriptManager(loop, database)


    async def handle(self):
        async with self.database.connect() as conn:
            result = await conn.execute(select([ScriptEntity]))
            for script in await result.fetchall():
                self.script_manager.schedule_script(
                    script.name,
                    script.script_contents,
                    script.crontab)

        self.cancel_reschedule()



