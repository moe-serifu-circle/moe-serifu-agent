import asyncio
import json
from msa.builtins.command.event import RegisterCommandEvent

from msa import supervisor

async def consumer_handler(consumer, websocket, path):
    # Use functools.partial to add consumer
    async for message in websocket:
        await consumer(message)

async def producer_handler(producer, websocket, path):
    # Use functools.partial to add producer
    while True:
        message = await producer()

        if message is not None:
            await websocket.send(message)

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



def serialize_event(event):
    for name, event_type in supervisor.registered_event_types.items():
        if isinstance(event, event_type) and not isinstance(event, RegisterCommandEvent):
            return json.dumps({
                "event_type": name,
                "data": event.data
            }).encode("UTF-8")
    return None

def deserialize_event(raw_event):
    raw_event = json.loads(raw_event)
    for name, event_type in supervisor.registered_event_types.items():
        if name == raw_event["event_type"]:
            new_event = event_type()
            new_event.load(raw_event["data"])
            new_event.propogateRemote = False
            return new_event



