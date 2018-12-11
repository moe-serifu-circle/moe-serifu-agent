import unittest
import logging
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock

from msa.core import supervisor
from msa.builtins.tty.handlers import TtyInputHandler, TtyOutputHandler
from msa.builtins.tty.events import TextInputEvent, TextOutputEvent, StyledTextOutputEvent


class TtyInputHandlerTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        dummy_logger = logging.getLogger('foo').addHandler(logging.NullHandler())
        self.handler = TtyInputHandler(loop=self.loop, event_queue=self.event_queue, logger=dummy_logger)

    @mock.patch("msa.builtins.tty.prompt.Prompt.listen", new=AsyncMock(return_value="test"))
    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, False, True]))
    def test_create_event_on_input(self):

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

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
        self.assertIsInstance(param_1, TextInputEvent)
        self.assertEqual(param_1.data.get("message"), "test")


class TtyOutputHandlerTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        dummy_logger = logging.getLogger('foo').addHandler(logging.NullHandler())
        self.handler = TtyOutputHandler(loop=self.loop, event_queue=self.event_queue, logger=dummy_logger)

    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch("msa.builtins.tty.handlers.TtyOutputHandler.print", new=mock.MagicMock())
    def test_print_text_on_text_output_event(self):

        raw_text = "test message"
        event = TextOutputEvent()
        event.init({
            "message": raw_text
        })
        self.event_queue.put_nowait((1, event))

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # stop the loop after 0.5 seconds
        self.loop.call_later(1, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.stop()
        self.loop.close()

        # get call arguments from fire_event mock
        call_args = self.handler.print.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        # Validate type and message
        self.assertEqual(param_1, raw_text)

    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch("msa.builtins.tty.handlers.TtyOutputHandler.print", new=mock.MagicMock())
    def test_print_text_on_styled_text_event(self):

        raw_text = "test message"
        event = StyledTextOutputEvent()
        event.init({
            "message": [
                {"text": raw_text,
                 "color": "blue",
                }]
        })
        self.event_queue.put_nowait((1, event))

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.stop()
        self.loop.close()

        # get call arguments from fire_event mock
        call_args = self.handler.print.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        # Validate type and message
        self.assertTrue(raw_text in param_1)


if __name__ == "__main__":
    unittest.main()

