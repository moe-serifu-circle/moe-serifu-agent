import asyncio
import math
from datetime import datetime, timedelta
from msa.core.event_handler import EventHandler
from msa.builtins.scripting import events
from msa.builtins.scripting.entities import ScriptEntity
from msa.builtins.scripting.script_execution_manager import ScriptExecutionManager
from msa.core import supervisor
from msa.builtins.signals import events as signal_events


class AddScriptHandler(EventHandler):
    """
    Handles AddScript Events
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.event_bus.subscribe(events.AddScriptEvent, self.handle_add_script_event)
        self.script_manager = ScriptExecutionManager(loop)

    async def handle_add_script_event(self, event):
        result = await ScriptEntity.filter(name=event.data["name"]).first()

        if not result:
            self.logger.debug(f"Script upload recieved for script \"{event.data['name']}\"")

            crontab = event.data.get("crontab", None)

            insert_new_script = ScriptEntity.create(
                name=event.data["name"],
                crontab=crontab,
                script_contents=event.data["script_contents"]
            )
            await insert_new_script

            # schedule script
            if crontab:
                self.logger.debug(f"Scheduling script \"{event.data['name']}\"")
                self.script_manager.schedule_script(
                    event.data["name"],
                    event.data["script_contents"],
                    crontab)
            else:
                self.logger.debug("Not scheduling uploaded script \"{event.data['name']} as crontabe is None.")

        else:
            self.logger.error(f"Script upload received for script \"{event.data['name']}\", but a script with this name already exists.")

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
    Handles TriggerScriptRunHandler Events
    """
    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_execution_manager = ScriptExecutionManager(loop)

        self.event_bus.subscribe(events.TriggerScriptRunEvent, self.handle_trigger_script_run_event)

        self.started = True

    async def handle_trigger_script_run_event(self, event):
        script = await ScriptEntity.filter(name=event.data["name"]).first()

        if script is not None:
            self.script_execution_manager.schedule_script(
                script.name,
                script.script_contents,
                None)
        else:
            self.logger.warn(f"Attempted to trigger script run for script \"{event.data['name']}\" but this script does not exist in the database.")

    
class StartupEventHandler(EventHandler):
    """
    Handles Startup Event to start the script manager
    """
    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_manager = ScriptExecutionManager(loop)

        self.event_bus.subscribe(signal_events.StartupEvent, self.handle_startup_event)

    async def handle_startup_event(self, event):
        for script in await ScriptEntity.all():
            if script.crontab is not None:
                self.script_manager.schedule_script(
                    script.name,
                    script.script_contents,
                    script.crontab)

        self.event_bus.unsubscribe(signal_events.StartupEvent, self.handle_startup_event)


class TriggerScriptListHandler(EventHandler):
    """
    Handles TriggerScriptListHandler Events
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_execution_manager = ScriptExecutionManager(loop)

        self.event_bus.subscribe(events.TriggerListScriptsEvent, self.handle_trigger_script_list_event)

    async def handle_trigger_script_list_event(self, event):
        scripts = []

        for script_entity in await ScriptEntity.all():
            if script_entity.name in self.script_execution_manager.scheduled_scripts:
                aiocron_instance = self.script_execution_manager.scheduled_scripts[script_entity.name]["aiocron_instance"]
                time_sec =  aiocron_instance.handle.when() + aiocron_instance.time
                scheduled_for = datetime.fromtimestamp(time_sec)

                scheduled_for = scheduled_for.strftime("%Y-%m-%dT%H:%M:%S.%f")
            else:
                scheduled_for = None

            script = {
                "id": script_entity.id,
                "name": script_entity.name,
                "crontab": script_entity.crontab,
                "created": script_entity.created.strftime("%Y-%m-%dT%H:%M:%S.%f"),
                "last_edited": script_entity.last_edited.strftime("%Y-%m-%dT%H:%M:%S.%f"),
                "last_run": script_entity.last_run.strftime("%Y-%m-%dT%H:%M:%S.%f")
                    if script_entity.last_run is not None
                    else None,
                "running": script_entity.name in self.script_execution_manager.running_scripts,
                "scheduled_for": scheduled_for,
            }

            scripts.append(script)

        new_event = events.ListScriptsEvent().init({
            "scripts": scripts
        })
        supervisor.fire_event(new_event)



class TriggerGetScriptHandler(EventHandler):
    """
    Handles :class:`TriggerGetScriptEvent` Events
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_execution_manager = ScriptExecutionManager(loop)

        self.event_bus.subscribe(events.TriggerGetScriptEvent, self.handle_trigger_get_script_event)

    async def handle_trigger_get_script_event(self, event):

        event_name = event.data["name"]

        script_entity = await ScriptEntity.filter(name=event_name).first()

        if script_entity.name in self.script_execution_manager.scheduled_scripts:
            aiocron_instance = self.script_execution_manager.scheduled_scripts[script_entity.name]["aiocron_instance"]
            time_sec =  aiocron_instance.handle.when() + aiocron_instance.time
            scheduled_for = datetime.fromtimestamp(time_sec)

            scheduled_for = scheduled_for.strftime("%Y-%m-%dT%H:%M:%S.%f")
        else:
            scheduled_for = None

        script = {
            "id": script_entity.id,
            "name": script_entity.name,
            "crontab": script_entity.crontab,
            "content": script_entity.script_contents,
            "created": script_entity.created.strftime("%Y-%m-%dT%H:%M:%S.%f"),
            "last_edited": script_entity.last_edited.strftime("%Y-%m-%dT%H:%M:%S.%f"),
            "last_run": script_entity.last_run.strftime("%Y-%m-%dT%H:%M:%S.%f"),
            "running": script_entity.name in self.script_execution_manager.running_scripts,
            "scheduled_for": scheduled_for,
        }


        new_event = events.GetScriptEvent().init(script)
        supervisor.fire_event(new_event)


class TriggerDeleteScriptHandler(EventHandler):
    """
    Handles :class:`TriggerDeleteScriptEvent` Events
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.script_execution_manager = ScriptExecutionManager(loop)

        self.event_bus.subscribe(events.TriggerDeleteScriptEvent, self.handle_trigger_delete_script_event)

    async def handle_trigger_delete_script_event(self, event):

        entity_name = event.data["name"]

        script_entity = await ScriptEntity.filter(name=entity_name).first()

        if script_entity is None:
            self.logger.debug(f"Could not find {entity_name} in database.")
            new_event = events.ScriptDeletedEvent().init({
                "name": entity_name,
                "status": "failure",
                "reason": f"Unable to find script with name {entity_name}."
            })
            supervisor.fire_event(new_event)
            return

        if script_entity.name in self.script_execution_manager.running_scripts:
            task = self.script_execution_manager.scheduled_scripts[script_entity.name]["task"]

            self.logger.debug(f"Canceling running script task {entity_name}.")
            task.cancel()

            try:
                await task
            except asyncio.CancelledError:
                self.logger.debug(f"Canceled script task {entity_name} successfully.")
            except:
                self.logger.debug(f"Failed to cancel script {entity_name} task.")
                new_event = events.ScriptDeletedEvent().init({
                    "name": entity_name,
                    "status": "failure",
                    "reason": f"Failed to cancel script with name {entity_name}."
                })
                supervisor.fire_event(new_event)
                return
        else:
            self.logger.debug(f"Script {entity_name} not running, so there is no task to cancel.")

        if script_entity.name in self.script_execution_manager.scheduled_scripts:
            del self.script_execution_manager.scheduled_scripts[script_entity.name]

        if script_entity.name in self.script_execution_manager.running_scripts:
            self.script_execution_manager.running_scripts.remove(script_entity.name)

        try:
            await script_entity.delete()
        except Exception as e:
            self.logger.warn(f"Failed to delete script {entity_name} from database. Exception: ", e)
            new_event = events.ScriptDeletedEvent().init({
                "name": entity_name,
                "status": "failure",
                "reason": f"Failed to cancel script with name {entity_name}."
            })
            supervisor.fire_event(new_event)
            return

        # record that the event has been successfully deleted
        new_event = events.ScriptDeletedEvent().init({
            "name": entity_name,
            "status": "success",
        })
        supervisor.fire_event(new_event)

















