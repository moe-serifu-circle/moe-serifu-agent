#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core import supervisor

def register_routes(route_binder):

    @route_binder.post("/conversation/talk")
    async def talk(payload):
        from msa.builtins.conversation.events import ConversationInputEvent
        new_event = ConversationInputEvent().init(payload)
        supervisor.fire_event(new_event)

        return {"text": f"Conversational input submitted."}












