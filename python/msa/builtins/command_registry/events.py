from schema import Schema, And

from msa.core.event import Event


class RegisterCommandEvent(Event):
    def __init__(self):
        super().__init__(
            priority=1,
            schema=Schema({
                "event_constructor": lambda e: issubclass(e, CommandEvent),
                "invoke": And(str, len),
                "describe": And(str, len),
                "usage": And(str, len),
                "options": And(str, len)
            })
        )


class CommandEvent(Event):
    def __init__(self, priority):
        super().__init__(
            priority=priority,
            schema=Schema({
                "raw_text": And(str, len),
                "tokens": [str]
            }))


class HelpCommandEvent(CommandEvent):
    def __init__(self):
        super().__init__(priority=9)

