from msa.event import Event


class CommandEventFactory:

    def __init__(self, event_constructor, invoke, describe, usage, options=None):
        self.event_constructor = event_constructor
        self.invoke = invoke
        self.describe = describe
        self.usage = usage
        self.options = options

    def create_event(self, data):
        return self.event_constructor(data)


class RegisterCommandEvent(Event):
    def __init__(self, command_event_factory):
        super().__init__(priority=5, data={
            "factory": command_event_factory
        })



