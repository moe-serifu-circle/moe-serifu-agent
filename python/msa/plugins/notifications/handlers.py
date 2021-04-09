from msa.core.event_handler import EventHandler
from msa.plugins.notifications.events import SendNotificationEvent
from msa.plugins.notifications.notification_providers import NotificationProvider
from msa.utils.asyncio_utils import sync_to_async

try:
    from notifiers import get_notifier
except:
    print("notifications plugin, requires the notifications_plugin extra to be installed.")
    exit()


class SendNotificationEventHandler(EventHandler):
    """
    Handles ConversationInputEvents
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)

        self.providers = {}

        for provider_type, provider_config in config["providers"].items():
            provider_type_enum = NotificationProvider[provider_type]
            if provider_type_enum == NotificationProvider.pushbullet:
                pushbullet = get_notifier("pushbullet")

                def send_notification(title, message, target):
                    pushbullet.notify(
                        title=title, message=message, token=provider_config["token"]
                    )

                self.providers[NotificationProvider.pushbullet] = sync_to_async(
                    send_notification
                )

            elif provider_type_enum == NotificationProvider.email:
                email = get_notifier("email")

                def send_notification(title, message, target):
                    email.notify(
                        subject=title,
                        message=message,
                        to=target,
                        from_=provider_config["from"],
                        username=provider_config.get("username"),
                        password=provider_config.get("password"),
                        ssl=provider_config.get("ssl", False),
                        tls=provider_config.get("tls", False),
                    )

                self.providers[NotificationProvider.email] = sync_to_async(
                    send_notification
                )

            elif provider_type_enum == NotificationProvider.slack:
                slack = get_notifier("slack")

                def send_notification(title, message, target):
                    slack.notify(message=f"{title}:\n{message}", channel=target)

                self.providers[NotificationProvider.slack] = sync_to_async(
                    send_notification
                )

        self.event_bus.subscribe(SendNotificationEvent, self.handle_event)

    async def handle_event(self, event):

        provider = event.data["provider"]
        provider_enum = NotificationProvider(provider)
        title = event.data["title"]
        message = event.data["message"]
        target = event.data.get("target", None)

        if provider_enum not in self.providers:
            self.logger.error(
                f"Incoming event refers to an unknown or a not configured provider: {provider}"
            )

        await self.providers[provider_enum](title, message, target)
