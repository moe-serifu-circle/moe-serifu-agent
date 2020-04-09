from msa.core import supervisor
import json


async def trigger_event(request):
    """

    :param request:
    :return:
    """
    from msa.core.event import Event
    new_event = Event.deserialize(request.payload)
    supervisor.fire_event(new_event)

    return {"text": f"Event {new_event} sucessfully triggered."}


async def get_events(request):
    """

    :param request:
    :return:
    """

    from msa.builtins.signals.events import RequestDisburseEventsToNetworkEvent, DisburseEventsToNetworkEvent

    new_event = RequestDisburseEventsToNetworkEvent().init({})
    supervisor.fire_event(new_event)

    response_event = await supervisor.listen_for_result(DisburseEventsToNetworkEvent)

    return response_event.get_metadata()


def register_routes(route_binder):

    route_binder.post("/signals/trigger_event")(trigger_event)
    route_binder.get("/signals/events")(get_events)
