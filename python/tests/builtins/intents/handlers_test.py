import asyncio
import unittest
from unittest.mock import patch, MagicMock

from msa.builtins.intents.handlers import IntentToEventHandler
from msa.builtins.intents.events import IntentEvent
from msa.core.event import Event
from tests.async_test_util import AsyncMock
from schema import Schema


class MyEvent(Event):
    def __init__(self):
        super().__init__(priority=50, schema=Schema({"prop_1": 1, "prop_2": "2"}))


class IntentToEventHandlerTest(unittest.TestCase):
    @patch("msa.core.supervisor_instance")
    @patch("msa.builtins.intents.handlers.Event.__subclasses__")
    def test_converting_intent_to_event(self, Event_subclass_mock, SupervisorMock):

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()
        handler = IntentToEventHandler(loop, event_bus_mock, logger_mock)

        MyEvent.__module__ = "my_module"

        Event_subclass_mock.return_value = [MyEvent]

        incoming_event = IntentEvent().init(
            {"type": "my_module.MyEvent", "context": {"prop_1": 1, "prop_2": "2"}}
        )

        loop.run_until_complete(handler.handle(incoming_event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]

        self.assertIsInstance(fired_event, MyEvent)
        self.assertIn("prop_1", fired_event.data)
        self.assertEqual(1, fired_event.data["prop_1"])
        self.assertIn("prop_2", fired_event.data)
        self.assertEqual("2", fired_event.data["prop_2"])

    @patch("msa.core.supervisor_instance")
    @patch("msa.builtins.intents.handlers.Event.__subclasses__")
    def test_class_not_found(self, Event_subclass_mock, SupervisorMock):

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()
        handler = IntentToEventHandler(loop, event_bus_mock, logger_mock)

        MyEvent.__module__ = "my_module"

        Event_subclass_mock.return_value = [MyEvent]

        incoming_event = IntentEvent().init(
            {"type": "my_module.MyOther", "context": {"prop_1": 1, "prop_2": "2"}}
        )

        loop.run_until_complete(handler.handle(incoming_event))

        SupervisorMock.fire_event.assert_not_called()

        logger_mock.warning.assert_called_once_with(
            f"Failed to translate Intent to Event. Could not find event: my_module.MyOther"
        )

    @patch("msa.core.supervisor_instance")
    @patch("msa.builtins.intents.handlers.Event.__subclasses__")
    def test_converted_event_fails_to_validate(
        self, Event_subclass_mock, SupervisorMock
    ):

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()
        handler = IntentToEventHandler(loop, event_bus_mock, logger_mock)

        MyEvent.__module__ = "my_module"

        Event_subclass_mock.return_value = [MyEvent]

        incoming_event = IntentEvent().init(
            {"type": "my_module.MyEvent", "context": {"prop_1": 1, "prop_2": 2}}
        )

        loop.run_until_complete(handler.handle(incoming_event))

        SupervisorMock.fire_event.assert_not_called()

        logger_mock.exception.assert_called_once_with(
            "While attempting to convert an intent to an event, the event initialization failed "
            "due to a schema validation error"
        )
