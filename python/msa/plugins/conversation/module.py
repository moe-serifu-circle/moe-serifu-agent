
from msa.plugins.conversation import event
from msa.plugins.conversation import coroutine
from msa.module import Module

class PluginModule(Module):

    coroutines = [
        coroutine.KeyboardInputCoroutine()
    ]


    def register(self):
        pass


    def unregister(self):
        pass

