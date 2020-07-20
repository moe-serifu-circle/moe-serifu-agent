import aiocron
import traceback
from functools import partial
import asyncio
from msa.api import get_api
from msa.api.context import ApiContext
from msa.builtins.scripting.entities import ScriptEntity
import logging
from datetime import datetime


class ScriptExecutionManager:
    __shared_state = None

    def __init__(self, loop=None):

        if ScriptExecutionManager.__shared_state is None:
            ScriptExecutionManager.__shared_state = {}
            self.__dict__ = ScriptExecutionManager.__shared_state

            root_logger = logging.getLogger("msa")
            self.logger = root_logger.getChild(
                "msa.builtins.scripting.script_execution_manager.ScriptManager"
            )

            self.loop = loop
            self.running_scripts = set()
            self.scheduled_scripts = {}

            self.local_api = get_api(ApiContext.local)

            self.globals = {"msa_api": self.local_api}
            self.func_locals = {}
            self.locals = {}

        else:
            self.__dict__ = ScriptExecutionManager.__shared_state

    async def aexec(self, identifier, code):
        self.logger.debug(f'Prepping execution of script "{identifier}"')
        # Make an async function with the code and `exec` it
        effective_globals = {**self.func_locals, **self.globals}
        func = (
            f"async def __ex(): "
            + "".join(f"\n {l}" for l in code.split("\n"))
            + "\n return locals()"
        )
        try:
            exec(func, effective_globals, self.locals)
        except:
            self.logger.error(f'Failed to prep execution of script "{identifier}"')

        self.logger.debug(f'Scheduling execution of script "{identifier}"')
        # Get `__ex` from local variables and call it
        task = asyncio.create_task(self.locals["__ex"]())

        self.logger.debug(f'Awaiting execution of script "{identifier}"')

        try:
            self.func_locals = await task
        except Exception as e:
            msg = traceback.format_exc()
            print(msg)
            self.logger.error(msg)

        for key in list(self.func_locals.keys()):
            if key in self.globals:
                del self.func_locals[key]
                raise Exception(
                    f'Statement attempted to override global variable "{key}". This is not allowed.'
                )

    async def run_script(self, name, script_content):
        # TODO capture log, errors, etc, and log to db via RunScriptResultEvent(Event):
        if self.scheduled_scripts[name]["aiocron_instance"] is not None:
            self.logger.debug(f'Scheduling cron execution of script "{name}"')
            while True:
                await self.scheduled_scripts[name]["aiocron_instance"].next()

                await self._record_script_run(name)

                self.running_scripts.add(name)
                self.logger.debug(f'Script "{name}" is about to execute')

                await self.aexec(name, script_content.strip())
                self.running_scripts.remove(name)
                self.logger.debug(f'Script "{name}" finished.')
        else:
            # run once and exit
            await self._record_script_run(name)
            self.running_scripts.add(name)
            await self.aexec("adhoc_scrpipt_<" + name + ">", script_content.strip())
            self.running_scripts.remove(name)
        self.script_finished(name)

    async def _record_script_run(self, name):
        try:
            instance = await ScriptEntity.filter(name=name).first()
            instance.last_run = datetime.now()
            await instance.save()
        except Exception as e:
            print(e)

    def schedule_script(self, name, script_content, crontab_definition=None):

        if name not in self.scheduled_scripts:
            script_coro = partial(self.run_script, name, script_content)

            self.scheduled_scripts[name] = {
                "task": self.loop.create_task(script_coro()),
                "aiocron_instance": aiocron.crontab(crontab_definition)
                if crontab_definition
                else None,
            }

    def script_finished(self, name):
        del self.scheduled_scripts[name]

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
