import asyncio
import traceback
from contextlib import suppress
import asyncio
import logging

class EventHandler:
    """The base event handler class, all other event handlers should be a subclass of this type.
    Properties:
    - self.loop (asyncio.AbstractEventLoop): the main event loop.
    - self.event_queue (asyncio.Queue): an event loop that this handler may attempt to read events out of by awaiting on
    it."""

    def __init__(self, loop: asyncio.AbstractEventLoop, event_queue: asyncio.Queue, logger: logging.Logger ):
        """Creates a new event handler. Subclasses should call the base constructor before setting up their own internal
        state.
        Params:
        - loop (asyncio.AbstractEventLoop): an asyncio event loop.
        - event_queue (asyncio.Queue): a queue that this handler may attempt to read events out of.
        - logger (logging.Logger): A logger instance specific to this event handler."""
        self.loop = loop
        self.event_queue = event_queue
        self.logger = logger

    async def init(self):
        """An optional initialization hook, may be used for executing setup code before all handlers have benn fully
        started."""
        pass

    async def handle_wrapper(self):
        """A method that wraps self.handle and handles repeatedly calling the handler while the system is still running.
        Called automatically by the supervisor during startup."""

        from msa.core import supervisor

        while not supervisor.should_stop():
            try:
                with suppress(asyncio.CancelledError):
                    await self.handle()
            except Exception as err:
                traceback.print_exc()
            await asyncio.sleep(0.01)


    async def handle(self):
        """An abstract method which must be overwritten. Once the system is started, the handle method will be called
        repeatedly until the system shuts down. The handler must be non-blocking."""
        raise NotImplementedError()




