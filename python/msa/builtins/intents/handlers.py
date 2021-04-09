#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core.event_handler import EventHandler
from msa.core import get_supervisor
from msa.builtins.intents.events import IntentEvent
from msa.core.event import Event


class IntentToEventHandler(EventHandler):
    """
    Handles IntentEvents and converts them to specific event types.

    This allows specific event types to be generated without placing a Type-dependency on a specific plugin or handler.
    When an Intent is received, if the plugin providing required event type is not loaded, no typed event will be
    propagated.
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.event_bus.subscribe(IntentEvent, self.handle)

    async def handle(self, event):

        event_type = event.data["type"]
        event_data = event.data.get("context", None)
        subclasses = Event.__subclasses__()

        found_cls = False
        self.logger.debug(
            f"Attempting to translate Intent to event of type: {event_type}"
        )
        for cls in subclasses:
            mod = f"{cls.__module__}.{cls.__name__}"
            self.logger.debug(f"Checking: {mod}")
            if event_type == mod:
                found_cls = True
                break

        if not found_cls:
            self.logger.warning(
                f"Failed to translate Intent to Event. Could not find event: {event_type}"
            )
            return

        new_event = cls().init(event_data)
        get_supervisor().fire_event(new_event)
