from schema import Schema

from msa.core.event import Event

class TextInputEvent(Event):
    def __init__(self):
        super().__init__(
            priority=5,
            schema=Schema({
                "message": str
            })
        )