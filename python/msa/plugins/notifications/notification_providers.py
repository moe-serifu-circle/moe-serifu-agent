from enum import Enum


class NotificationProvider(Enum):
    pushbullet = "pushbullet"
    email = "email"
    slack = "slack"
