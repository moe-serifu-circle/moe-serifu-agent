import unittest
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock

from msa.core import supervisor
from msa.builtins.command.events import QuitCommandEvent
from msa.builtins.command.handlers import QuitHandler


class QuitTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        self.handler = QuitHandler(loop=self.loop, event_queue=self.event_queue)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch('msa.core.supervisor.stop', new=mock.Mock())
    def test_quit(self):

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle_wrapper())

        # Create quit event
        event = QuitCommandEvent()
        raw_text = "quit"
        event.init({
            "raw_text": raw_text,
            "tokens": raw_text.split()[1:]
        })
        self.event_queue.put_nowait((10, event))  # ensure lower priority

        # stop the loop after 0.5 seconds
        self.loop.call_later(1, lambda:self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.close()

        # ensure supervisor's stop method was called
        supervisor.stop.assert_called()


if __name__ == "__main__":
    unittest.main()
