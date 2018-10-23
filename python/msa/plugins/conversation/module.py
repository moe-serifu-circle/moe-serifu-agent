
from msa.plugins.conversation import event
from msa.plugins.conversation import coroutine
from msa.modes import Modes

class PluginModule:

    coroutines = [
        coroutine.ConversationCoroutine()
    ]

    events = {
        "ConverseCommandEvent": event.ConverseCommandEvent
    }

    allowed_modes = [
        Modes.cli,
        Modes.server
    ]
