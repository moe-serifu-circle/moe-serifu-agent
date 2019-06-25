from msa.core import supervisor


def register_routes(route_binder):

    @route_binder.post("/scripting/script")
    async def add_script(request=None, raw_data=None):

        if raw_data:
            data = raw_data
        else:
            data = await dict(request.post())

        from msa.builtins.scripting.events import AddScriptEvent
        new_event = AddScriptEvent().init(data)
        supervisor.fire_event(new_event)












