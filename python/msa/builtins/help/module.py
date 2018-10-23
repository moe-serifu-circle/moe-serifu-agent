
from msa.builtins.help import coroutine
from msa.builtins.help import event

class PluginModule:

    coroutines = [
        coroutine.HelpCoroutine()
    ]

    events = {
        "HelpCommandEvent": event.HelpCommandEvent
    }
