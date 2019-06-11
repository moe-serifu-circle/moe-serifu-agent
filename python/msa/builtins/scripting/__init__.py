from msa.builtins.scripting import handlers
from msa.builtins.scripting import entities

handler_factories = [
    handlers.AddScriptHandler,
]

entity_setup = entities.setup

entities = [
    entities.Script,
    entities.ScriptRunResult,
]
