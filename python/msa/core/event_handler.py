import asyncio
from typing import Dict, Optional
import logging

import msa
from msa.core.event_bus import EventBus


class EventHandler:
    """The base event handler class, all other event handlers should be a subclass of this type.

    Attributes
    ----------
    self.loop : asyncio.AbstractEventLoop
        the main event loop.
    self.event_queue : asyncio.Queue
        an event loop that this handler may attempt to read events out of by awaiting on
        it."""

    def __init__(
        self,
        loop: asyncio.AbstractEventLoop,
        event_bus: EventBus,
        logger: logging.Logger,
        config: Optional[Dict] = None,
    ):
        """Creates a new event handler. Subclasses should call the base constructor before setting up their own internal
        state.

        Parameters
        ----------
        loop : asyncio.AbstractEventLoop
            an asyncio event loop.
        event_bus : msa.core.event_bus.EventBus
            An instance of the event bus that the handler can use to subscribe to events.
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
