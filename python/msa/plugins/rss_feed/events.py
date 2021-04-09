from msa.core.event import Event
from schema import Schema


class RssFeedRequestEvent(Event):
    """
    RssFeedRequestEvent schema:
    - timestamp: datetime in yyyy-mm-dd hh:mm:ss:xx format of starup event
    """

    def __init__(self):
        super().__init__(priority=40, schema=Schema(None))
