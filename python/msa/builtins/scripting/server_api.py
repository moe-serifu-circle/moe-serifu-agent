from msa.core import supervisor


def register_routes(route_binder):

    @route_binder.post("/scripting/script")
    async def add_script(connection_type, payload):
        from msa.builtins.scripting.events import AddScriptEvent
        new_event = AddScriptEvent().init(payload)
        supervisor.fire_event(new_event)

        return {"text": f"{payload['name']} was sucessfully uploaded"}












