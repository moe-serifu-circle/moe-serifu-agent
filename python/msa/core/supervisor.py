import sys
import os
import asyncio
import traceback
import logging
import inspect
from contextlib import suppress
from concurrent.futures import ThreadPoolExecutor

from msa.core.loader import load_builtin_modules, load_plugin_modules
from msa.core.event_bus import EventBus
from msa.core.config_manager import ConfigManager
from msa.server.route_adapter import RouteAdapter
from msa.api import MsaLocalApiWrapper
from msa.data import __models__


class Supervisor:
    """The supervisor is responsible for managing the execution of the application and orchestrating the event system.
    """

    def __init__(self):
        self.config_manager = None
        self.stop_loop = False
        self.stop_main_coro = None
        self.stop_future = None


        self.loaded_modules = []

        self.initialized_event_handlers = []

        self.handler_lookup = {}

        self.shutdown_callbacks = []

        self.executor = ThreadPoolExecutor()

        self.root_logger = None
        self.logger = None
        self.loggers = {}

    def init_logging(self, logging_config):
        """
        Initializes application logging, setting up the global log namespace, and the supervisor log namespace.
        """

        self.root_logger = logging.getLogger("msa")
        self.root_logger.setLevel(logging_config["global_log_level"])

        mode = "w" if logging_config["truncate_log_file"] else "a"
        file_handler = logging.FileHandler(logging_config["log_file_location"], mode=mode)
        file_handler.setLevel(logging.DEBUG)

        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(name)s - %(message)s')

        file_handler.setFormatter(formatter)
        self.root_logger.addHandler(file_handler)

        self.logger = self.root_logger.getChild("core.supervisor")
        self.loggers["core.supervisor"] = self.logger

    def apply_granular_log_levels(self, granular_level_config):
        """
        Applies the granular log levels configured in the conficuration file.

        Parameters
        ----------
        granular_level_config : List[Dict[String, String]]
            A list of namespace to log level mappings to be applied.
        """
        self.logger.info("Setting granular log levels.")

        # order by length shortest namespaces will be the highest level, and should be applied first so that lower
        # level rules may be applied without being overwritten.
        granular_level_config = sorted(granular_level_config, key=lambda e: len(e["namespace"]), reverse=True)

        for log_config in granular_level_config:
            for namespace, logger in self.loggers.items():
                if log_config["namespace"] in namespace:
                    effective_log_level = log_config.get("level", logger.getEffectiveLevel())
                    logger.setLevel(effective_log_level)

        self.logger.info("Finished setting granular log levels.")

    def init(self, loop, cli_config, route_adapter):
        """Initializes the supervisor.

        Parameters
        ----------
        loop : Asynio Event Loop    
            An asyncio event loop the supervisor should use.
        cli_config: Dict
            A dictionary containing configuration options derived from the command line interface.
        route_adapter: ** fix docstrings **
        """
        if not os.environ.get("TEST"):
            self.loop = loop 
            self.event_bus = EventBus(self.loop)
            self.event_queue = asyncio.Queue(self.loop)
            # block getting a loop if we are running unit tests
            # helps suppress a warning.


        client_api_binder= MsaLocalApiWrapper()
        server_api_binder = route_adapter

        # ### PLACEHOLDER - Load Configuration file here --
        self.config_manager = ConfigManager(cli_config)
        config = self.config_manager.get_config()

        # Initialize logging
        self.init_logging(config["logging"])

        plugin_names = []

        # ### Loading Modules
        self.logger.info("Loading modules.")

        # load builtin modules
        self.logger.debug("Loading builtin modules.")
        bultin_modules = load_builtin_modules()
        self.logger.debug("Finished loading builtin modules.")

        # load plugin modules
        self.logger.debug("Loading plugin modules.")
        plugin_modules = load_plugin_modules(plugin_names)
        self.logger.debug("Finished loading plugin modules.")

        self.logger.info("Finished loading modules.")

        self.loaded_modules = bultin_modules + plugin_modules


        # ### Registering Handlers
        self.logger.info("Registering handlers.")
        for module in self.loaded_modules:
            self.logger.debug("Registering client api endpoints for module msa.{}".format(module.__name__))
            if hasattr(module, "register_client_api") and callable(module.register_client_api):
                module.register_client_api(client_api_binder)

            self.logger.debug("Registering server api endpoints for module msa.{}".format(module.__name__))
            if hasattr(module, "register_server_api") and callable(module.register_server_api):
                module.register_server_api(server_api_binder)

            if hasattr(module, "entities") and isinstance(module.entities, list):
                __models__.extend(module.entities)

            self.logger.debug("Registering handlers for module msa.{}".format(module.__name__))
            for handler in module.handler_factories:

                namespace = "{}.{}".format(module.__name__[4:], handler.__name__)
                full_namespace = "msa.{}".format(namespace)
                self.logger.debug("Registering handler: msa.{}".format(namespace))

                handler_logger = self.root_logger.getChild(namespace)
                self.loggers[full_namespace] = handler_logger

                module_config = config["module_config"].get(full_namespace, None)

                inited_handler = handler(self.loop, self.event_bus, handler_logger, module_config)

                self.initialized_event_handlers.append(inited_handler)
                self.handler_lookup[handler] = inited_handler

                self.logger.debug("Finished registering handler: {}".format(full_namespace))
            self.logger.debug("Finished registering handlers for module {}".format(module.__name__))

        client_api_binder.freeze_registration()
        self.logger.info("Finished registering handlers.")

        self.apply_granular_log_levels(config["logging"]["granular_log_levels"])


    def start(self, additional_coros=[]):
        r"""Starts the supervisor.

        Parameters
        ----------
        additional_coros : List[Coroutines]
            a list of other coroutines to be started. Acts as a hook for specialized
            startup scenarios.
        """

        self.logger.info("Starting main coroutine.")

        try:
            with suppress(asyncio.CancelledError):
                self.logger.debug("Priming main coroutine")
                primed_coro = self.main_coro(additional_coros)
                self.logger.debug("Main coroutine primed, executing in the loop.")
                self.loop.create_task(primed_coro)
                self.logger.debug("Finished running main coroutine.")
        #except KeyboardInterrupt:
        #   self.logger.info("Keyboard interrupt (Ctrl-C) encountered, beginning shutdown.")
        #   print("Ctrl-C Pressed. Quitting...")
        finally:
            pass
            #self.stop()
            #self.loop.run_until_complete(self.loop.shutdown_asyncgens())
            #self.logger.info("Stopping loop.")
            #self.loop.close()
            #self.logger.info("Exiting.")
            #sys.exit(0)

    def stop(self):
        """Schedules the supervisor to stop, and exit the application."""
        self.logger.info("Schedule the main coroutine to stop.")
        self.stop_future = asyncio.ensure_future(self.exit())

    async def exit(self):
        """Shuts down running tasks and stops the event loop, exiting the application."""
        self.logger.info("Stopping handlers, and main coroutine.")
        self.stop_loop = True

        self.logger.debug("Shutting down executor threads.")
        self.executor.shutdown()

        self.logger.debug("Calling shutdown callbacks.")
        for callback in self.shutdown_callbacks:
            callback()

        await asyncio.sleep(0.5) # let most of the handlers finish their current loop
        await asyncio.sleep(0.5) # let most of the handlers finish their current loop

        self.stop_main_coro = True

        self.logger.debug("Cancel any remaining tasks.")
        if sys.version_info[0] == 3 and sys.version_info[1] == 6:
            pending = asyncio.Task.all_tasks()
            current = asyncio.Task.current_task()
        else:
            pending = asyncio.all_tasks() # get all tasks
            current = asyncio.current_task()

        pending.remove(current)  # except this task
        pending.remove(self.main_coro_task)

        for task in pending:
            if not task.done():
                with suppress(asyncio.CancelledError):
                    task.cancel()
                    await asyncio.sleep(0.01)
                    await task



    def fire_event(self, new_event):
        """Fires an event to all event listeners.

        Parameters
        ----------
        new_event : `Event`
            A new instance of a subclass of `Event` to be propagated to other event handlers.
        """
        self.logger.debug("Fire event: {}".format(new_event))
        def fire():
            self.loop.create_task(self.event_bus.fire_event(new_event))


        self.loop.call_soon(fire)


    async def main_coro(self, additional_coros=[]):
        """The main coroutine that manages starting the handlers, and waiting for a shutdown signal.

        Parameters
        ----------
        additional_coros : List[Coroutines]
            Additional coroutines to be run in the event loop.

        """
        self.logger.debug("Main coroutine executing.")

        if sys.version_info[0] == 3 and sys.version_info[1] == 6:
            self.main_coro_task = asyncio.Task.current_task()
        else:
            self.main_coro_task = asyncio.current_task()


        # ### Initialize database requirements for modules
        self.logger.info("Initializing database add-ons")
        for module in self.loaded_modules:
            if hasattr(module, "entity_setup"):
                self.logger.debug(f"Initializing database for module msa.{module.__name__}")
                await module.entity_setup()
                self.logger.debug(f"Finished initializing database for module msa.{module.__name__}")

        self.logger.debug("Main Coro: Call async init on handlers.")
        init_coros = [
            handler.init()
            for handler in self.initialized_event_handlers
        ]
        await asyncio.gather(*init_coros)

        self.logger.debug("Main Coro: Prime handler coroutines.")
        primed_coros = [
            handler.handle_wrapper()
            for handler in self.initialized_event_handlers
        ]

        self.logger.debug("Main Coro: Prime additional coroutines: {}".format(len(additional_coros)))
        if len(additional_coros) > 0:
            primed_coros.extend(additional_coros)

        futures = None

        try:
            self.logger.debug("Beginning handler execution.")
            with suppress(asyncio.CancelledError):
                futures = await asyncio.gather(*primed_coros)
        except Exception as err:
            self.logger.error(err, traceback.print_exc())

        self.logger.debug("Main Coro: Sleep until shutdown is started.")
        while not self.stop_main_coro:
            await asyncio.sleep(0.5)

        self.logger.debug("Main Coro: Wakeup for shutdown.")

        if futures is not None:
            for future in futures:
                if future is not None:
                    with suppress(asyncio.CancelledError):
                        await future

        # cancel and suppress exit future
        if self.stop_future is not None:
            asyncio.gather(self.stop_future)

        self.logger.info("Main coro: finishing execution.")

        print("\rGoodbye!\n")






    def should_stop(self):
        """Indicates whether the supervisor is in the process is shutting down.
        Used for signaling event_handlers to cancel rescheduling.
        """
        return self.stop_loop

    def get_handler(self, handler_type):
        """Returns the handler instance for a given type of handler. Used for unit tests.

        Parameters
        ----------
        - handler_type: A type of handler."""
        return self.handler_lookup.get(handler_type)

