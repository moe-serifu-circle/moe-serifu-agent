from msa.core import supervisor
import json


def register_routes(route_binder):

    @route_binder.post("/signals/trigger_event")
    def add_script(payload):

        from msa.core.event import Event
        new_event = Event.deserialize(payload)
        supervisor.fire_event(new_event)

        return {"text": f"Event {new_event} sucessfully triggered."}












