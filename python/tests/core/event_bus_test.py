import unittest
from unittest import mock

from msa.core.event_bus import EventBus
from msa.core.event import Event
from tests.async_test_util import AsyncMock, async_run

class EventBusTest(unittest.TestCase):

    def setUp(self):
        self.event_bus = EventBus()

    @mock.patch("asyncio.PriorityQueue.put_nowait", new=AsyncMock())
    def test_fire_event(self):

        event_queue = self.event_bus.create_event_queue()

        new_event = Event(priority=0, schema={})

        async_run(self.event_bus.fire_event(new_event))

        event_queue.put_nowait.mock.assert_called_once()


if __name__ == '__main__':
    unittest.main()
