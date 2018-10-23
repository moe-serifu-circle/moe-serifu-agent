from msa.event import Event


class CommandEventFactory:

    def __init__(self, event_constructor, invoke, describe, usage, options=None):
        self.event_constructor = event_constructor
        self.invoke = invoke
        self.describe = describe
        self.usage = usage
        self.options = options

    def create_event(self, data):
        new_event = self.event_constructor()
        new_event.init(data)
        return new_event


class RegisterCommandEvent(Event):
    def __init__(self):
        super().__init__(priority=5)

    def init(self, command_event_factory):
        self.data = {
            "factory": command_event_factory
        }


