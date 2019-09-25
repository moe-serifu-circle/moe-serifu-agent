from functools import partial
import asyncio
import aiocron

from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.scripting import events
from msa.builtins.scripting.entities import ScriptEntity
from msa.builtins.signals import events as signal_events
from msa.api import MsaLocalApiWrapper


class ScriptManager:
    __shared_state = None
    def __init__(self, loop=None):

        if ScriptManager.__shared_state is None:
            ScriptManager.__shared_state = {}
            self.__dict__ = ScriptManager.__shared_state

            self.loop = loop
            self.running_scripts = {}

            self.local_api = MsaLocalApiWrapper().get_api()

            self.globals = {
                "msa_api":  self.local_api
            }
        else:
            self.__dict__ = ScriptManager.__shared_state



    async def run_script(self, name, script_content, crontab_definition=None):
        # TODO capture log, errors, etc, and log to db via RunScriptResultEvent(Event):
        if crontab_definition is not None:
            while True:
                await aiocron.crontab(crontab_definition).next()
                exec(script_content.strip(), self.globals, {})
        else:
            # run once and exit
            exec(script_content.strip(), self.globals, {})

        self.script_finished(name)

    def schedule_script(self, name, script_content, crontab_definition=None):
        if name not in self.running_scripts:

            script_coro = partial(self.run_script, name, script_content, crontab_definition)

            self.running_scripts[name] = self.loop.create_task(script_coro())

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

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.event_bus.subscribe(events.AddScriptEvent, self.handle_add_script_event)
        self.script_manager = ScriptManager(loop)

    async def handle_add_script_event(self, event):
        result = await ScriptEntity.filter(name=event.data["name"]).first()

        if not result:
            self.logger.debug(f"Script upload recieved for script \"{event.data['name']}\"")

            insert_new_script = ScriptEntity.create(
                name=event.data["name"],
                crontab=event.data["crontab"],
                script_contents=event.data["script_contents"]
            )
            await insert_new_script

            # schedule script
            self.logger.debug(f"Scheduling script \"{event.data['name']}\"")
            self.script_manager.schedule_script(
                event.data["name"],
                event.data["script_contents"],
                event.data["crontab"])

        else:
            self.logger.error(f"Script upload recieved for script \"{event.data['name']}\", but a script with this name already exists.")

            new_event = events.AddScriptFailedEvent()
            new_event.init({

                    "error": f"Failed to add script {event.data['name']}." ,
                    "description": "A script with this name already exists.",
                    "description_verbose": ("A script with this name already exists. Delete the script with "
                                            "this name then try again, or change the name of the script you are attempting to upload"),
            })
            supervisor.fire_event(
                new_event
            )
                

class TriggerScriptRunHandler(EventHandler):
    """
    Handles TriggerScriptRun Events
    """
    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_manager = ScriptManager(loop )

        self.event_bus.subscribe(events.TriggerScriptRunEvent, self.handle_trigger_script_run_event)

        self.started = True

    async def handle_trigger_script_run_event(self, event):
        print("hi", event)
        script = await ScriptEntity.filter(name=event.data["name"]).first()

        if script:
            self.script_manager.schedule_script(
                script.name,
                script.script_contents,
                None)
        else:
            self.logger.warn(f"Attempted to trigger script run for script \"{script.name}\" but this script does not exist in the database.")

    
class StartupEventHandler(EventHandler):
    """
    Handles Startup Event to start the script manager
    """
    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_manager = ScriptManager(loop)

        self.event_bus.subscribe(signal_events.StartupEvent, self.handle_startup_event)

    async def handle_startup_event(self, event):
        for script in await ScriptEntity.all():
            self.script_manager.schedule_script(
                script.name,
                script.script_contents,
                script.crontab)

        self.event_bus.unsubscribe(signal_events.StartupEvent, self.handle_startup_event)




