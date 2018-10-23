
from msa.builtins.quit import coroutine
from msa.builtins.quit import event

class PluginModule:

    coroutines = [
        coroutine.QuitCoroutine()
    ]

    events = {
        "QuitCommandEvent": event.QuitCommandEvent
    }

