import asyncio
import unittest
from unittest import mock

from msa.core.event_bus import EventBus
from msa.core.event import Event
from tests.async_test_util import AsyncMock, async_run

class EventBusTest(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_bus = EventBus(self.loop)

    @mock.patch("asyncio.PriorityQueue.put_nowait", new=mock.Mock())
    def test_fire_event(self):

        event_queue = self.event_bus.create_event_queue()

        new_event = Event(priority=0, schema={})

        async_run(self.loop, self.event_bus.fire_event(new_event))

        event_queue.put_nowait.assert_called()

        self.loop.stop()
        self.loop.close()


if __name__ == '__main__':
    unittest.main()
