from schema import Schema, And, Or, Optional
from msa.core.event import Event


class IntentEvent(Event):
    """
    IntentEvent schema:
    - type: The type of the intent. This should be the full namespace path of the event that should be created if the
        appropriate plugin is loaded.
    - context: The data that the new event should be initialized with.
    """

    def __init__(self):
        super().__init__(
            priority=50,
            schema=Schema({"type": And(str, len), Optional("context"): Or(dict, None)}),
        )
