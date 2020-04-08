from enum import Enum, auto


class ConnectionType(Enum):
    local = auto()
    rest = auto()
    websocket = auto()

