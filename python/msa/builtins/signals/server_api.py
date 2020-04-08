from msa.core import supervisor
import json


def register_routes(route_binder):

    @route_binder.post("/signals/trigger_event")
    async def trigger_event(connection_type, payload):

        from msa.core.event import Event
        new_event = Event.deserialize(payload)
        supervisor.fire_event(new_event)

        return {"text": f"Event {new_event} sucessfully triggered."}


    @route_binder.get("/signals/events")
    async def get_events(connection_type, payload):

        from msa.builtins.signals.events import RequestDisburseEventsToNetworkEvent, DisburseEventsToNetworkEvent

        new_event = RequestDisburseEventsToNetworkEvent().init({})
        supervisor.fire_event(new_event)

        response_event = await supervisor.listen_for_result(DisburseEventsToNetworkEvent)

        return response_event.get_metadata()


