import asyncio
import sys
import signal
from contextlib import suppress
import importlib

from msa.core.loader import load_builtin_modules, load_plugin_modules
from msa.core.event_bus import EventBus

class Supervisor:
    """The supervisor is responsible for managing the execution of the application and orchestrating the event system.
    """

    def __init__(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.Queue()
        self.stop_loop = False
        self.stop_main_coro = None
        self.stop_future = None

        self.event_bus = EventBus(self.loop)

        self.loaded_modules = []

        self.initialized_event_handlers = []

        self.handler_lookup = {}

        self.shutdown_callbacks = []


    def init(self, mode):
        """Initializes the supervisor.
        Params:
        - mode (int): A msa.core.RunMode enum value to configure which modules should be started based on the
        environment the system is being run in.
        """

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

                inited_handler = handler(self.loop, event_queue)

                self.initialized_event_handlers.append(inited_handler)
                self.handler_lookup[handler] = inited_handler

    def start(self, additional_coros=[]):
        """Starts the supervisor.
        Params:
        - additional_coros (List[Coroutines]): a list of other coroutines to be started. Acts as a hook for specialized
        startup scenarios."""


        try:
            with suppress(asyncio.CancelledError):
                primed_coro = self.main_coro(additional_coros)
                self.loop.run_until_complete(primed_coro)
        except KeyboardInterrupt:
            print("Ctrl-C Pressed. Quitting...")
        finally:
            self.stop()

    def stop(self):
        """Schedules the supervisor to stop, and exit the application."""
        self.stop_future = asyncio.ensure_future(self.exit())

    async def exit(self):
        """Shuts down running tasks and stops the event loop, exiting the application."""
        self.stop_loop = True

        for callback in self.shutdown_callbacks:
            callback()

        await asyncio.sleep(1) # let most of the handlers finish their current loop

        self.stop_main_coro = True

        pending = asyncio.all_tasks() # get all tasks
        pending.remove(asyncio.current_task()) # except this task

        for task in pending:
            with suppress(asyncio.CancelledError):
                task.cancel()
                await task

    def fire_event(self, new_event):
        """Fires an event to all event listeners."""
        def fire():
            self.loop.create_task(self.event_bus.fire_event(new_event))


        self.loop.call_soon(fire)


    async def main_coro(self, additional_coros=[]):
        """The main coroutine that manages starting the handlers, and waiting for a shutdown signal."""

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

    def get_handler(self, handler_type):
        """"Returns the handler instance for a given type of handler. Used for unit tests.
        Params:
        - handler_type: A type of handler."""
        return self.handler_lookup.get(handler_type)

