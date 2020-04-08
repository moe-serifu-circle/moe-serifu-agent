#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core import supervisor
from msa.api import ApiContext

def register_routes(route_binder):

    @route_binder.post("/conversation/talk")
    async def talk(connection_type, payload):
        from msa.builtins.conversation.events import ConversationInputEvent, ConversationOutputEvent
        new_event = ConversationInputEvent().init(payload)
        supervisor.fire_event(new_event)

        if connection_type is not ApiContext.websocket:
            response_event = await supervisor.listen_for_result(ConversationOutputEvent)
            return {"text": response_event.data["output"]}












