import asyncio
import sys
from contextlib import suppress
import importlib

from msa.core.loader import load_builtin_modules, load_plugin_modules
from msa.core.event_bus import EventBus

class Supervisor:

    def __init__(self):
        self.loop = asyncio.get_event_loop()
        self.event_queue = asyncio.Queue()
        self.stop_loop = False
        self.stop_main_coro = None
        self.stop_future = None

        self.event_bus = EventBus()

        self.loaded_modules = []

        self.initialized_event_handlers = []


    def init(self, mode):
        plugin_names = []


        # load builtin modules
        bultin_modules = load_builtin_modules()

        # load plugin modules
        plugin_modules = load_plugin_modules(plugin_names, mode)

        self.loaded_modules = bultin_modules + plugin_modules

        # register event handlers
        for module in self.loaded_modules:
            for handler in module.handler_factories:

                event_queue = self.event_bus.create_event_queue()

                inited_coro = handler(self.loop, event_queue)

                self.initialized_event_handlers.append(inited_coro)

    def start(self, additional_coros=[]):

        try:
            with suppress(asyncio.CancelledError):
                primed_coro = self.main_coro(additional_coros)
                self.loop.run_until_complete(primed_coro)
        finally:
            self.loop.close()

    def stop(self):
        self.stop_future = asyncio.ensure_future(self.exit())

    async def exit(self):
        self.stop_loop = True

        for callback in self.shutdown_callbacks:
            callback()

        await asyncio.sleep(1) # let most of the handlers finish their current loop

        self.stop_main_coro = True

        pending = asyncio.Task.all_tasks() # get all tasks
        pending.remove(asyncio.Task.current_task()) # except this task

        for task in pending:
            with suppress(asyncio.CancelledError):
                task.cancel()
                await task

    async def fire_event(self, new_event):
        await self.event_bus.fire_event(new_event)


    async def main_coro(self, additional_coros=[]):
        # "paralellizes" tasks, scheduling them on the event loop

        primed_coros = [
            handler.handle_wrapper()
            for handler in self.initialized_event_handlers
        ]

        if len(additional_coros) > 0:
            primed_coros.extend(additional_coros)

        futures = asyncio.gather(*primed_coros, return_exceptions=True)

        while not self.stop_main_coro:
            await asyncio.sleep(0.5)

        with suppress(asyncio.CancelledError):
            await futures

        # cancel and suppress exit future
        if self.stop_future is not None:
            with suppress(asyncio.CancelledError):
                self.stop_future.cancel()
                await self.stop_future



    def should_stop(self):
        """Indicates whether the supervisor is in the process is shutting down.
        Used for signaling event_handlers to cancel rescheduling.
        """
        return self.stop_loop
