from msa.core import supervisor

async def add_script(request):
    """

    :param request:
    :return:
    """
    from msa.builtins.scripting.events import AddScriptEvent
    new_event = AddScriptEvent().init(request.payload)
    supervisor.fire_event(new_event)

    return {"text": f"{request.payload['name']} was sucessfully uploaded"}


def register_routes(route_binder):

    route_binder.post("/scripting/script")(add_script)
