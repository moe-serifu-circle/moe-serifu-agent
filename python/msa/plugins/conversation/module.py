
from msa.plugins.conversation import event
from msa.plugins.conversation import coroutine

class PluginModule:

    coroutines = [
        coroutine.ConversationCoroutine()
    ]

    events = []

