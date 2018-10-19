
from msa.builtins.terminal_input import event
from msa.builtins.terminal_input import coroutine


class PluginModule:

    coroutines = [
        coroutine.KeyboardInputCoroutine()
    ]

    events = [
        event.TextInputEvent
    ]





