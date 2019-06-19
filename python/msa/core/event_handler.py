import asyncio
from typing import Dict, Optional
import traceback
from contextlib import suppress
import asyncio
import logging

import msa

class EventHandler:
    """The base event handler class, all other event handlers should be a subclass of this type.

    Attributes
    ----------
    self.loop : asyncio.AbstractEventLoop
        the main event loop.
    self.event_queue : asyncio.Queue
        an event loop that this handler may attempt to read events out of by awaiting on
        it."""

    def __init__(self, loop: asyncio.AbstractEventLoop, event_bus: msa.core.event_bus.EventBus, database, logger: logging.Logger,
                 config: Optional[Dict] = None):
        """Creates a new event handler. Subclasses should call the base constructor before setting up their own internal
        state.

        Parameters
        ----------
        loop : asyncio.AbstractEventLoop
            an asyncio event loop.
        event_bus : msa.core.event_bus.EventBus
            An instance of the event bus that the handler can use to subscribe to events.
        database : sqlalchemy database engine
            An instance of a sql alchemy database engine the handler can use for database tasks.
        logger : logging.Logger
            A logger instance specific to this event handler."""
        self.loop = loop
        self.event_bus = event_bus
        self.logger = logger
        self.config = config

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




