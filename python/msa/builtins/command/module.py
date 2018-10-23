
from msa.builtins.command import coroutine
from msa.builtins.command import event
from msa.modes import Modes

class PluginModule:

    coroutines = [
        coroutine.CommandCoroutine()
    ]

    events = {
        "RegisterCommandEvent": event.RegisterCommandEvent
    }

    allowed_modes = [
        Modes.cli,
        Modes.server
    ]




