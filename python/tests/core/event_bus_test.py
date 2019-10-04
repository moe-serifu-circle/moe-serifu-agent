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

    def test_fire_event_no_subscriber(self):
        fake_handler = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler.callback)

        new_event = FakeEventB()
        async def main():
            await self.event_bus.fire_event(new_event)
            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        self.loop.stop()
        self.loop.close()

        assert not fake_handler.callback_called
        assert fake_handler.event == None

    def test_fire_event_one_subscriber(self):

        fake_handler = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler.callback)

        new_event = FakeEventA()

        async def main():
            await self.event_bus.fire_event(new_event)
            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert fake_handler.callback_called
        assert fake_handler.event == new_event

    def test_fire_event_multiple_subscribers(self):

        fake_handler_1 = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler_1.callback)

        fake_handler_2 = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler_2.callback)

        new_event = FakeEventA()

        async def main():
            await self.event_bus.fire_event(new_event)
            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert fake_handler_1.callback_called
        assert fake_handler_1.event == new_event
        
        assert fake_handler_2.callback_called
        assert fake_handler_2.event == new_event

    def test_fire_event_multiple_subscribers_different_events(self):

        fake_handler_1 = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler_1.callback)

        fake_handler_2 = FakeHandler()
        self.event_bus.subscribe(FakeEventB, fake_handler_2.callback)

        new_event_a = FakeEventA()
        new_event_b = FakeEventB()

        async def main():
            await self.event_bus.fire_event(new_event_a)
            await self.event_bus.fire_event(new_event_b)
            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert fake_handler_1.callback_called
        assert fake_handler_1.event == new_event_a
        
        assert fake_handler_2.callback_called
        assert fake_handler_2.event == new_event_b


    def test_unsubscribe(self):

        fake_handler_1 = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler_1.callback)
        self.event_bus.unsubscribe(FakeEventA, fake_handler_1.callback)

        new_event_a = FakeEventA()

        async def main():
            await self.event_bus.fire_event(new_event_a)
            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert not fake_handler_1.callback_called
        assert fake_handler_1.event == None

    def test_unsubscribe_after_event_is_fired(self):

        fake_handler_1 = FakeHandler()
        self.event_bus.subscribe(FakeEventA, fake_handler_1.callback)

        new_event_a = FakeEventA()

        async def main():
            await self.event_bus.fire_event(new_event_a)

            self.event_bus.unsubscribe(FakeEventA, fake_handler_1.callback)

            await self.event_bus.listen(timeout=0.1)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert not fake_handler_1.callback_called
        assert fake_handler_1.event == None
        
    def test_complex_subscribers(self):

        fake_handler_1 = FakeHandler()
        fake_handler_2 = FakeHandler()

        self.event_bus.subscribe(".*A", fake_handler_1.callback)
        self.event_bus.subscribe(".*B", fake_handler_2.callback)

        new_event_a = FakeEventA()
        new_event_b = FakeEventB()

        async def main():
            await self.event_bus.fire_event(new_event_a)
            await self.event_bus.fire_event(new_event_b)
            await self.event_bus.listen(timeout=0.2)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert fake_handler_1.callback_called
        assert fake_handler_1.event == new_event_a

        assert fake_handler_2.callback_called
        assert fake_handler_2.event == new_event_b

    def test_complex_subscribers_two_subscribers_same_event(self):

        fake_handler_1 = FakeHandler()
        fake_handler_2 = FakeHandler()

        self.event_bus.subscribe(".*A", fake_handler_1.callback)
        self.event_bus.subscribe(".*A", fake_handler_2.callback)

        new_event_a = FakeEventA()

        async def main():
            await self.event_bus.fire_event(new_event_a)
            await self.event_bus.listen(timeout=0.2)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert fake_handler_1.callback_called
        assert fake_handler_1.event == new_event_a

        assert fake_handler_2.callback_called
        assert fake_handler_2.event == new_event_a

    def test_complex_unsubscribe(self):

        fake_handler_1 = FakeHandler()
        fake_handler_2 = FakeHandler()

        self.event_bus.subscribe(".*A", fake_handler_1.callback)
        self.event_bus.unsubscribe(".*A", fake_handler_1.callback)
        self.event_bus.subscribe(".*B", fake_handler_2.callback)

        new_event_a = FakeEventA()
        new_event_b = FakeEventB()

        async def main():
            await self.event_bus.fire_event(new_event_a)
            await self.event_bus.fire_event(new_event_b)
            await self.event_bus.listen(timeout=0.2)

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

        async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert not fake_handler_1.callback_called
        assert fake_handler_1.event == None

        assert fake_handler_2.callback_called
        assert fake_handler_2.event == new_event_b


    def test_listen_for_result(self):

        new_event = FakeEventA()

        async def main():

            def insert():
                self.event_bus.queue.put_nowait((new_event.priority, new_event))

            self.loop.call_soon(insert)

            task_results = await asyncio.gather(
                self.event_bus.listen_for_result(FakeEventA),
                self.event_bus.listen(timeout=0.1),
            )

            result = task_results[0]

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

            return result

        result = async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert result == new_event

    def test_listen_for_result_twice_for_same_event_type(self):

        new_event = FakeEventA()

        async def main():

            def insert():
                self.event_bus.queue.put_nowait((new_event.priority, new_event))

            self.loop.call_soon(insert)

            task_results = await asyncio.gather(
                self.event_bus.listen_for_result(FakeEventA),
                self.event_bus.listen_for_result(FakeEventA),
                self.event_bus.listen(timeout=0.1),
            )

            result_1 = task_results[0]
            result_2 = task_results[0]

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

            return result_1, result_2

        result_1, result_2 = async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert result_1 == new_event
        assert result_2 == new_event

    def test_listen_for_result_with_timeout(self):

        new_event = FakeEventA()

        async def main():

            def insert():
                self.event_bus.queue.put_nowait((new_event.priority, new_event))

            self.loop.call_soon(insert)

            task_results = await asyncio.gather(
                self.event_bus.listen_for_result(FakeEventA, timeout=0.5),
                self.event_bus.listen(timeout=0.1),
            )

            result = task_results[0]

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

            return result

        result = async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert result == new_event
    
    def test_listen_for_result_with_timeout_and_actually_timeout(self):

        new_event = FakeEventA()

        async def main():

            task_results = await asyncio.gather(
                self.event_bus.listen_for_result(FakeEventA, timeout=0.1),
                self.event_bus.listen(timeout=0.1),
            )

            result = task_results[0]

            await asyncio.sleep(0)
            await asyncio.sleep(0)
            await asyncio.sleep(0)

            return result

        result = async_run(self.loop, main())

        self.loop.stop()
        self.loop.close()

        assert result == None

### Helpers

class FakeEventA(Event):
    def __init__(self):
        super().__init__(priority=0, schema={})

class FakeEventB(Event):
    def __init__(self):
        super().__init__(priority=0, schema={})

class FakeHandler():
    def __init__(self):
        self.event = None
        self.callback_called = False
    async def callback(self, event):
        self.event = event
        self.callback_called = True
        return "callback result"

if __name__ == '__main__':
    unittest.main()
