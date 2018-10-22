
from msa.builtins.quit import coroutine

class PluginModule:

    coroutines = [
        coroutine.QuitCoroutine()
    ]

