from msa.builtins.scripting import handlers
from msa.builtins.scripting import entities

handler_factories = [
    handlers.AddScriptHandler,
    handlers.StartupEventHandler,
]

entity_setup = entities.setup

entities = [
    entities.ScriptEntity,
    entities.ScriptRunResultEntity,
]
