import asyncio


async def consumer_handler(consumer, websocket, path):
    # Use functools.partial to add consumer
    async for message in websocket:
        await consumer(message)

async def producer_handler(producer, websocket, path):
    # Use functools.partial to add producer
    async for message in websocket:
        await producer(message)

async def handler(connected, consumer_handler, producer_handler, websocket, path):
    # use functools.partial to add reference to connected set
    connected.add(websocket)

    try:
        consumer_task = asyncio.ensure_future(
            consumer_handler(websocket, path))
        producer_task = asyncio.ensure_future(
            producer_handler(websocket, path))
        done, pending = await asyncio.wait(
            [consumer_task, producer_task],
            return_when=asyncio.FIRST_COMPLETED,
        )
        for task in pending:
            task.cancel()
    finally:
        connected.remove(websocket)

