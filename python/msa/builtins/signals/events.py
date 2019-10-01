import datetime
from schema import Schema, And, Or

from msa.core.event import Event


class StartupEvent(Event):
    """
    StartEvent schema:
    - timestamp: datetime in yyyy-mm-dd hh:mm:ss:xx format of starup event
    """

    def __init__(self):
        super().__init__(
            priority=0,
            schema=Schema({
                "timestamp": And(str, len)
            })
        )


class RequestDisburseEventsToNetworkEvent(Event):
    """
    RequestDisburseEventsToClientEvent schema:
    """

    def __init__(self):
        super().__init__(
            priority=0,
            schema=Schema({
            })
        )

class DisburseEventsToNetworkEvent(Event):
    """
    DisburseEventsToNetworkEvent schema:
    """

    def __init__(self):
        super().__init__(
            priority=0,
            schema=Schema({
                "events": [
                    dict
                ]
            })
        )
