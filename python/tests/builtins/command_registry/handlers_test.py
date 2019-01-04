import unittest
import logging
from unittest import mock
import asyncio
import sys

from tests.async_test_util import AsyncMock

from msa.core import supervisor
from msa.builtins.tty.events import TextInputEvent
from msa.builtins.command_registry import CommandRegistryHandler, HelpCommandHandler
from msa.builtins.command_registry.events import  CommandEvent, RegisterCommandEvent, HelpCommandEvent




class CommandRegistryTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        dummy_logger = logging.getLogger('foo')
        dummy_logger.addHandler(logging.NullHandler())
        self.handler = CommandRegistryHandler(loop=self.loop, event_queue=self.event_queue, logger=dummy_logger)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, False, True]))
    def test_register_and_invoke_command(self):

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # Create register event
        register_event = RegisterCommandEvent()
        register_event.init(data={
            "event_constructor": DummyCommandEvent,
            "invoke": "dummy",
            "describe": "a dummy event",
            "usage": "dummy",
            "options": "no options"
        })
        self.event_queue.put_nowait((0, register_event))

        # Create text input evvent
        raw_text = "dummy param_1 param_2"
        event = TextInputEvent()
        event.init({
            "message": raw_text
        })
        self.event_queue.put_nowait((10, event))  # ensure lower priority

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda:self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.close()

        # get call arguments from fire_event mock
        call_args = supervisor.fire_event.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        # Validate type and message
        self.assertIsInstance(param_1, DummyCommandEvent)
        self.assertEqual(param_1.data.get("tokens"), ["param_1", "param_2"])
        self.assertEqual(param_1.data.get("raw_text"), raw_text)


class HelpCommandTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        dummy_logger = logging.getLogger('foo').addHandler(logging.NullHandler())
        self.handler = HelpCommandHandler(loop=self.loop, event_queue=self.event_queue, logger=dummy_logger)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_init(self):

        # add init to execution loop
        self.loop.create_task(self.handler.init())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()

        # get call arguments from fire_event mock
        call_args = supervisor.fire_event.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        # Validate type and message
        self.assertIsInstance(param_1, RegisterCommandEvent)
        self.assertEqual(param_1.data.get("invoke"), "help")

    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, False, True]))
    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.command_registry.handlers.HelpCommandHandler.print')
    def test_handle_help_dummy_command(self, mocked_print):

        # Create register event
        register_event = RegisterCommandEvent()
        register_event.init(data={
            "event_constructor": DummyCommandEvent,
            "invoke": "dummy",
            "describe": "a dummy event",
            "usage": "dummy",
            "options": "no options"
        })
        self.event_queue.put_nowait((0, register_event))

        async def create_event():
            event = HelpCommandEvent()
            event.init({
                "raw_text": "help dummy",
                "tokens": ["dummy"],
            })
            self.event_queue.put_nowait((10, event))
        self.loop.create_task(create_event())

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # stop the loop after 0.5 seconds
        self.loop.call_later(1, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.stop()

        self.handler.print.assert_called()

        # get call arguments from fire_event mock
        call_args = self.handler.print.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        self.assertTrue("Help text for command 'dummy'" in param_1, param_1)

    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, False, True]))
    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.command_registry.handlers.HelpCommandHandler.print')
    def test_handle_help_dummy_command(self, mocked_print):

        # Create register event
        register_event = RegisterCommandEvent()
        register_event.init(data={
            "event_constructor": DummyCommandEvent,
            "invoke": "dummy",
            "describe": "a dummy event",
            "usage": "dummy",
            "options": "no options"
        })
        self.event_queue.put_nowait((0, register_event))

        async def create_event():
            event = HelpCommandEvent()
            event.init({
                "raw_text": "help",
                "tokens": [],
            })
            self.event_queue.put_nowait((10, event))
        self.loop.create_task(create_event())

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.close()

        self.handler.print.assert_called()

        # get call arguments from fire_event mock
        call_args = self.handler.print.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        self.assertTrue("Available Commands:" in param_1[0].get("text"), param_1)



class DummyCommandEvent(CommandEvent):
    def __init__(self):
        super().__init__(0)

if __name__ == "__main__":
    unittest.main()
