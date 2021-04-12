import asyncio
import unittest
from unittest.mock import patch, MagicMock

from msa.plugins.notifications.handlers import SendNotificationEventHandler
from msa.plugins.notifications.events import SendNotificationEvent
from msa.core.event import Event
from tests.async_test_util import AsyncMock
from schema import Schema


class IntentToEventHandlerTest(unittest.TestCase):
    @patch("msa.core.supervisor_instance")
    @patch("msa.plugins.notifications.handlers.get_notifier")
    def test_converting_intent_to_event(self, get_notifier_mock, SupervisorMock):

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()
        config = {
            "preferred_provider": "pushbullet",
            "providers": {
                "pushbullet": {"token": "abcd1234"},
                "email": {
                    "host": "http://test.com",
                    "port": 0000,
                    "from": "test@test.com",
                },
                "slack": {"webhook_url": "http://slack.com/api/asdfasdfadf-fake"},
            },
        }

        # set up notify mocks

        notifiers = {
            "pushbullet": MagicMock(),
            "email": MagicMock(),
            "slack": MagicMock(),
        }

        call_args = {
            "pushbullet": [
                "test",
                "test message",
                config["providers"]["pushbullet"]["token"],
            ],
            "email": [
                "test",
                "test message",
                "test_target",
                config["providers"]["email"]["from"],
                None,
                None,
                False,
                False,
            ],
            "slack": ["test:\ntest_message" "test_target"],
        }

        get_notifier_mock.side_effect = lambda k: notifiers[k]

        handler = SendNotificationEventHandler(
            loop, event_bus_mock, logger_mock, config
        )

        events_data = [
            {
                "provider": prov,
                "title": "test",
                "message": "test message",
                "target": "test_target",
            }
            for prov in ["pushbullet", "email", "slack"]
        ]

        events = [SendNotificationEvent().init(data) for data in events_data]

        for event in events:
            loop.run_until_complete(handler.handle_notify(event))

        for name, notifier in notifiers.items():
            notifier.notify.called_once_with(*call_args[name])
