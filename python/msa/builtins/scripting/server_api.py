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


async def list_scripts(request):
    """
    Lists scripts that are available to run on the daemon.

    :param request: The details of the incoming request.
    :type request: :class:`msa.server.route_adapter.ServerRequest`
    :return: A response to send to the client
    :rtype: :class:`Dict` or :class:`NoneType`
    """

    from msa.builtins.scripting.events import TriggerListScriptsEvent, ListScriptsEvent
    new_event = TriggerListScriptsEvent().init(None)
    supervisor.fire_event(new_event)

    response_event = await supervisor.listen_for_result(ListScriptsEvent)
    return {"scripts": response_event.data["scripts"]}

async def get_script(request):
    """
    Fetch a script that has been uploaded to the daemon and return it as a string

    :param request: The details of the incoming request.
    :type request: :class:`msa.server.route_adapter.ServerRequest`
    :return: A response to send to the client
    :rtype: :class:`Dict` or :class:`NoneType`
    """

    from msa.builtins.scripting.events import TriggerGetScriptEvent, GetScriptEvent
    new_event = TriggerGetScriptEvent().init({
        "name":  request.url_variables["name"]
    })
    supervisor.fire_event(new_event)

    response_event = await supervisor.listen_for_result(GetScriptEvent)
    return {"script": response_event.data}


def register_routes(route_binder):

    route_binder.post("/scripting/script")(add_script)
    route_binder.get("/scripting/script")(list_scripts)
    route_binder.get("/scripting/script/{name}")(get_script)
