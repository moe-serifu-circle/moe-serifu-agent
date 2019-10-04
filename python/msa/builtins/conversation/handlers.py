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

        input = self.normalize(event.data["input"])

        if input == "hello" or input == "hi":
            output = "Hello,  how are you?"

        elif "how are you" in input:
            output = "I am well thank you. What can I do for you?"

        else:
            output =  "I am afraid I don't know what to say."

        new_event = (events.ConversationOutputEvent()
                     .init({"output": output})
                     .network_propagate())
        supervisor.fire_event(new_event)

    def normalize(self, string):
        return string.strip().lower().replace("\n", "")
