import asyncio
import logging
import unittest
from unittest import mock

from tests.async_test_util import async_run
from msa.core.event_handler import EventHandler

class EventHandlerTest(unittest.TestCase):


    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.dummy_logger = logging.getLogger('foo')
        self.dummy_logger.addHandler(logging.NullHandler())
        self.event_bus = FakeEventBus()

    def test_constructor(self):

        self.event_handler = DummyEventHandler(self.loop, self.event_bus, self.dummy_logger)

        assert self.event_handler.loop == self.loop
        assert self.event_handler.event_bus == self.event_bus
        assert self.event_handler.logger == self.dummy_logger

    def test_init_hook(self):
        self.event_handler = DummyEventHandler(self.loop, self.event_bus, self.dummy_logger)
        async_run(self.loop, self.event_handler.init())



class DummyEventHandler(EventHandler):
    def __init__(self, loop, queue, logger):
        super().__init__(loop, queue, logger)

        self.read_from_queue = []


    async def handle(self):
        data = await self.event_queue.get()
        self.read_from_queue.append(data)

class FakeEventBus():
    pass

if __name__ == "__main__":
    unittest.main()
