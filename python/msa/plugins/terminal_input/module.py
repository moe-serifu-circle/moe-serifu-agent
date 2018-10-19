
from msa.plugins.terminal_input import event
from msa.plugins.terminal_input import coroutine


class PluginModule:

    coroutines = [
        coroutine.KeyboardInputCoroutine()
    ]

    events = [
        event.TextInputEvent
    ]





