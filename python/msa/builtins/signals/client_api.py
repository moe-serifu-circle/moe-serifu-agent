import requests


def register_endpoints(api_binder):

    @api_binder.register_method()
    def trigger_event(self, event):

        if not event.network_propagate:
            print("WARNING: event.network_propagate is not True, cancelling network propagation.")

        response = self.rest_client.post(
            "/signals/trigger_event",
            json=event.get_metadata())

        if not response:
            return
        if response.status_code != 200:
            raise Exception(response.raw)
        print(response.text)

