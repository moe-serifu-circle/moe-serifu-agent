from msa.builtins.scripting import handlers
from msa.builtins.scripting import entities
from msa.builtins.scripting import server_api
from msa.builtins.scripting import client_api

handler_factories = [
    handlers.AddScriptHandler,
    handlers.StartupEventHandler,
    handlers.TriggerScriptRunHandler,
    handlers.TriggerScriptListHandler,
    handlers.TriggerGetScriptHandler,
    handlers.TriggerDeleteScriptHandler,
]


entities = [
    entities.ScriptEntity,
    entities.ScriptRunResultEntity,
]

register_client_api = client_api.register_endpoints
register_server_api = server_api.register_routes
