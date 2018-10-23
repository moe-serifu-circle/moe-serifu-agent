
from msa.builtins.quit import coroutine
from msa.builtins.quit import event
from msa.modes import Modes

class PluginModule:

    coroutines = [
        coroutine.QuitCoroutine()
    ]

    events = {
        "QuitCommandEvent": event.QuitCommandEvent
    }

    allowed_events = [
        Modes.cli,
        Modes.server,
        Modes.client
    ]
