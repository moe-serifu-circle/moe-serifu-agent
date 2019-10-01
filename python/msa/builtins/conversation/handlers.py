#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.conversation import events
from msa.builtins.signals import events as signal_events

class ConversationInputEventHandler(EventHandler):
    """
    Handles ConversatoinInputEvents
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)
        self.event_bus.subscribe(events.ConversationInputEvent, self.handle)

    async def handle(self, event):
        # consider 
        # - https://github.com/gunthercox/ChatterBot
        # - https://github.com/huggingface/transformers

        new_event = (events.ConversationOutputEvent()
                     .init({"output": f"I am afraid I don't know what to say."})
                     .network_propagate())
        supervisor.fire_event(new_event)
