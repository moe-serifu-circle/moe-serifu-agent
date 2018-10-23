
from msa.builtins.command import coroutine
from msa.builtins.command import event

class PluginModule:

    coroutines = [
        coroutine.CommandCoroutine()
    ]

    events = {
        "RegisterCommandEvent": event.RegisterCommandEvent
    }



