import unittest
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock

from msa.core import supervisor
from msa.builtins.echo.events import EchoCommandEvent
from msa.builtins.echo.handlers import EchoHandler

import time


class EchoTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        self.handler = EchoHandler(loop=self.loop, event_queue=self.event_queue)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch('msa.builtins.echo.handlers.EchoHandler.print')
    def test_echo(self, mocked_print):

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle())

        # Create echo event
        raw_text = "echo sometext"
        event = EchoCommandEvent()
        event.init({
            "raw_text": raw_text,
            "tokens": raw_text.split()[1:]
        })
        self.event_queue.put_nowait((10, event))  # ensure lower priority

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda:self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.close()


        # get call arguments from fire_event mock
        self.handler.print.assert_called()
        call_args = self.handler.print.call_args
        text = call_args[0][0]

        # Validate message
        self.assertEqual(text, "sometext")


if __name__ == "__main__":
    unittest.main()
