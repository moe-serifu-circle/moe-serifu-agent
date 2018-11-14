import unittest
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock

from msa.core import supervisor
from msa.builtins.tty.handlers import TtyInputHandler
from msa.builtins.tty.events import TextInputEvent




class TtyInputHandlerTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        self.handler = TtyInputHandler(loop=self.loop, event_queue=self.event_queue)

    @mock.patch("prompt_toolkit.PromptSession.prompt", new=AsyncMock(return_value="test"))
    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    def test_create_event_on_input(self):

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda:self.loop.stop())

        # begin running loop
        self.loop.run_forever()

        # get call arguments from fire_event mock
        call_args = supervisor.fire_event.call_args

        # assert that we are able to get the TextInputEvent we were expecting
        self.assertIsNotNone(call_args)
        self.assertTrue(len(call_args[0]))
        param_1 = call_args[0][0]

        # Validate type and message
        self.assertIsInstance(param_1, TextInputEvent)
        self.assertEqual(param_1.data.get("message"), "test")


if __name__ == "__main__":
    unittest.main()