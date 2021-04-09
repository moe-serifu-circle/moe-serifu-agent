#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core import get_supervisor
from msa.api import ApiContext
from msa.server.server_response import ServerResponseText, ServerResponseType


async def talk(request):
    """

    :param request:
    :return:
    """
    from msa.builtins.conversation.events import (
        ConversationInputEvent,
        ConversationOutputEvent,
    )

    new_event = ConversationInputEvent().init(request.data).source(request.source)

    supervisor = get_supervisor()
    supervisor.fire_event(new_event)

    response_event = await supervisor.listen_for_result(ConversationOutputEvent)
    return ServerResponseText(ServerResponseType.success, response_event.data["output"])


def register_routes(route_binder):

    route_binder.post("/conversation/talk")(talk)
