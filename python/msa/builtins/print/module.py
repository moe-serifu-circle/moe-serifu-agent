
from msa.builtins.print import coroutine
from msa.builtins.print import event


class PluginModule:

    coroutines = [
        coroutine.PrintCoroutine()
    ]

    events = {
        "PrintTextEvent": event.PrintTextEvent
    }






