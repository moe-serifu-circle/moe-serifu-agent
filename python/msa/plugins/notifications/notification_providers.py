from enum import Enum, auto


class NotificationProvider(Enum):
    pushbullet = auto()
    email = auto()
    slack = auto()
