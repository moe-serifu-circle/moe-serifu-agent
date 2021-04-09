from schema import Schema, And, Or, Optional, Use
from msa.core.event import Event
from msa.plugins.notifications.notification_providers import NotificationProvider


class SendNotificationEvent(Event):
    """
    SendNotificationEvent schema:
    - timestamp: datetime in yyyy-mm-dd hh:mm:ss:xx format of starup event
    """

    def __init__(self):
        super().__init__(
            priority=40,
            schema=Schema(
                {
                    "provider": Or(
                        NotificationProvider.slack.value,
                        NotificationProvider.email.value,
                        NotificationProvider.pushbullet.value,
                    ),
                    Optional("target"): And(str, len),
                    "title": And(str, len),
                    "message": And(str, len),
                }
            ),
        )


class SendPreferredNotificationEvent(Event):
    """
    SendNotificationEvent schema:
    - timestamp: datetime in yyyy-mm-dd hh:mm:ss:xx format of starup event
    """

    def __init__(self):
        super().__init__(
            priority=40,
            schema=Schema(
                {
                    Optional("target"): And(str, len),
                    "title": And(str, len),
                    "message": And(str, len),
                }
            ),
        )
