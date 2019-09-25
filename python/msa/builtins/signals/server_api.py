from msa.core import supervisor
import json


def register_routes(route_binder):

    @route_binder.post("/signals/trigger_event")
    async def add_script(request=None, raw_data=None):

        if raw_data:
            data = raw_data
        else:
            data = json.loads((await request.read()).decode("utf-8"))

        from msa.core.event import Event
        new_event = Event.deserialize(data)
        supervisor.fire_event(new_event)

        return {"text": f"Event {new_event} sucessfully triggered."}












