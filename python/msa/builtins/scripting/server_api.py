from msa.core import get_supervisor
from msa.server.server_response import (
    ServerResponseText,
    ServerResponseType,
    ServerResponseJson,
)


async def add_script(request):
    """

    :param request:
    :return:
    """
    from msa.builtins.scripting.events import AddScriptEvent

    new_event = AddScriptEvent().init(request.data)
    get_supervisor().fire_event(new_event)

    return ServerResponseText(
        ServerResponseType.success, f"{request.data['name']} was successfully uploaded"
    )


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
    get_supervisor().fire_event(new_event)

    response_event = await get_supervisor().listen_for_result(ListScriptsEvent)

    return ServerResponseJson(
        ServerResponseType.success, payload={"scripts": response_event.data["scripts"]}
    )


async def get_script(request):
    """
    Fetch a script that has been uploaded to the daemon and return it as a string

    :param request: The details of the incoming request.
    :type request: :class:`msa.server.route_adapter.ServerRequest`
    :return: A response to send to the client
    :rtype: :class:`Dict` or :class:`NoneType`
    """

    from msa.builtins.scripting.events import TriggerGetScriptEvent, GetScriptEvent

    new_event = TriggerGetScriptEvent().init({"name": request.url_variables["name"]})
    get_supervisor().fire_event(new_event)

    response_event = await get_supervisor().listen_for_result(GetScriptEvent)

    return ServerResponseJson(
        ServerResponseType.success, payload={"script": response_event.data}
    )


async def delete_script(request):
    """
    Delete a script that has been uploaded to the daemon, cancelling it if it is running or scheduled to run.

    :param request: The details of the incoming request.
    :type request: :class:`msa.server.route_adapter.ServerRequest`
    :return: A response to send to the client
    :rtype: :class:`Dict` or :class:`NoneType`
    """

    from msa.builtins.scripting.events import (
        TriggerDeleteScriptEvent,
        ScriptDeletedEvent,
    )

    new_event = TriggerDeleteScriptEvent().init({"name": request.url_variables["name"]})
    get_supervisor().fire_event(new_event)

    response_event = await get_supervisor().listen_for_result(ScriptDeletedEvent)

    return ServerResponseJson(ServerResponseType.success, payload=response_event.data)


def register_routes(route_binder):

    route_binder.post("/scripting/script")(add_script)
    route_binder.get("/scripting/script")(list_scripts)
    route_binder.get("/scripting/script/{name}")(get_script)
    route_binder.delete("/scripting/script/{name}")(delete_script)
