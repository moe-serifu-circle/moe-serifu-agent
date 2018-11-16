from schema import Schema, Optional

from msa.core.event import Event

class TextInputEvent(Event):
    def __init__(self):
        super().__init__(
            priority=5,
            schema=Schema({
                "message": str
            })
        )

class TextOutputEvent(Event):
    def __init__(self):
        super().__init__(
            priority=5,
            schema=Schema({
                "message": str
            })
        )


class StyledTextOutputEvent(Event):
    def __init__(self):
        super().__init__(
            priority=5,
            schema=Schema({
                "message": [
                    {
                        "text": str,
                        "color": str,
                        Optional("attrib"): [str]
                    }
                ]
            })
        )
