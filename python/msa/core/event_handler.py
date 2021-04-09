import asyncio
from typing import Dict, Optional, List, Tuple, Callable, Coroutine
import logging
from datetime import datetime


import msa
from msa.core.event_bus import EventBus


class EventHandler:
    """The base event handler class, all other event handlers should be a subclass of this type.

    Attributes
    ----------
    loop : asyncio.AbstractEventLoop
        the main event loop.
    event_bus : msa.core.event_bus.EventBus
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

    def schedule(
        self,
    ) -> List[Tuple[str, Callable[[], Coroutine[datetime, None, None]]]]:
        """An optional hook, may be used for scheduling one or more methods/functions to be periodically called..

        The expected return value is a list of tuples. Each tuple should be a crontab string, followed by a coroutine
        that should be executed periodically based on the given crontab.

        Invalid crontabs will result in a warning in the log, and the coroutine will not be scheduled.

        Returns
        ------
        List[Tuple[str, Callable[[], Coroutine[None]]]]
        """
        return []

    schedule.base_class = True
