
from msa.builtins.help import coroutine
from msa.builtins.help import event
from msa.modes import Modes

class PluginModule:

    coroutines = [
        coroutine.HelpCoroutine()
    ]

    events = {
        "HelpCommandEvent": event.HelpCommandEvent
    }

    allowed_modes = [
        Modes.cli,
        Modes.server
    ]
