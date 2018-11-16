import asyncio
import unittest
from unittest import mock

from msa.core.event_handler import EventHandler

class EventHandlerTest(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue()
        self.event_handler = DummyEventHandler(self.loop, self.event_queue)

    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[i >= 20 for i in range(21)]))
    def test_handle(self):

        nums_10 = list(range(10))
        nums_20 = list(range(10, 20))

        # insert numbers 0-9
        for i in nums_10:
            self.loop.create_task(self.insert_into_queue(i))

        # queue handler
        self.loop.create_task(self.event_handler.handle_wrapper())

        # insert some more numbers
        for i in nums_20:
            self.loop.create_task(self.insert_into_queue(i))

        # queue shutdown later (should be more than enough time to finish work)
        self.loop.call_later(1, lambda: self.loop.stop())

        # run loop and wait for it to stop
        self.loop.run_forever()
        self.loop.close()

        # verify we read all the numbers
        self.assertEqual(nums_10 + nums_20, self.event_handler.read_from_queue)




    async def insert_into_queue(self, n):
        self.event_queue.put_nowait(n)

class DummyEventHandler(EventHandler):
    def __init__(self, loop, queue):
        super().__init__(loop, queue)

        self.read_from_queue = []


    async def handle(self):
        data = await self.event_queue.get()
        self.read_from_queue.append(data)

if __name__ == "__main__":
    unittest.main()
