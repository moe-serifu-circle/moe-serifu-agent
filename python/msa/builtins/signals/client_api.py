
from msa.core.event import Event

def register_endpoints(api_binder):

    @api_binder.register_method()
    async def trigger_event(self, event):

        if not event.network_propagate:
            print("WARNING: event.network_propagate is not True, cancelling network propagation.")

        response = await self.client.post(
            "/signals/trigger_event",
            payload=event.get_metadata())

        if not response:
            return
        if response.status_code != 200:
            raise Exception(response.raw)


    @api_binder.register_method()
    async def get_events(self):
        response = await self.client.get("/signals/events")

        if not response: 
            return
        if response.status_code != 200:
            raise Exception(response.raw)

        # load response json
        json = response.json()

        # load disburse event
        disburse_event = Event.deserialize(json)

        # load sent events
        deserialized_events = []
        for raw_event in disburse_event.data["events"]:
            new_event = Event.deserialize(raw_event)
            deserialized_events.append(new_event)

        return deserialized_events
