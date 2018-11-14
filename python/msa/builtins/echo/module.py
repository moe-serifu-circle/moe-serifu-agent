
from msa.builtins.echo import coroutine
from msa.builtins.echo import event
from msa.modes import Modes


class PluginModule:

    coroutines = [
        coroutine.EchoCoroutine()
    ]

    events = {
        "EchoCommandEvent": event.EchoCommandEvent
    }

    allowed_modes = [
        Modes.cli,
        Modes.server
    ]
