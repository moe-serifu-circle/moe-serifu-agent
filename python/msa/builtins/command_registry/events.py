from schema import Schema, And, Optional

from msa.core.event import Event


class RegisterCommandEvent(Event):
    """Used for registering a new command type with the Command Registry."""
    def __init__(self):
        super().__init__(
            priority=1,
            schema=Schema({
                "event_constructor": lambda e: issubclass(e, CommandEvent),
                "invoke": And(str, len),
                "describe": And(str, len),
                "usage": And(str, len),
                Optional("options", default="No available options."): And(str, len)
            })
        )


class CommandEvent(Event):
    """The base class for all command based events. All event constructors registered with the CommandRegistry must be
    as subclass of this class."""
    def __init__(self, priority):
        super().__init__(
            priority=priority,
            schema=Schema({
                "raw_text": And(str, len),
                "tokens": [str]
            }))


class HelpCommandEvent(CommandEvent):
    """A command event handled by the HelpCommandHandler, prompting it to print help text based on parameters provided.
    """
    def __init__(self):
        super().__init__(priority=9)

