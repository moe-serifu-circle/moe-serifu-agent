from msa.core import supervisor


def register_routes(route_binder):

    @route_binder.post("/scripting/script")
    def add_script(payload):
        from msa.builtins.scripting.events import AddScriptEvent
        new_event = AddScriptEvent().init(payload)
        supervisor.fire_event(new_event)

        return {"text": f"{data['name']} was sucessfully uploaded"}












