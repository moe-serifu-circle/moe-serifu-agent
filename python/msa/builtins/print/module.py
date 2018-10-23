
from msa.builtins.print import coroutine
from msa.builtins.print import event
from msa.modes import Modes


class PluginModule:

    coroutines = [
        coroutine.PrintCoroutine()
    ]

    events = {
        "PrintTextEvent": event.PrintTextEvent
    }


    allowed_modes = [
        Modes.cli,
        Modes.server,
        Modes.client
    ]





