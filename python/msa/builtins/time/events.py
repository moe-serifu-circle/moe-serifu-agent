import datetime

from schema import Schema

from msa.core.event import Event

class TimeEvent(Event):

    def __init__(self):
        super().__init__(
            priority=100,
            schema=Schema({
                "current_time": float
            })
        )
