import functools
import asyncio

from msa.remote.generic_producer_consumer import handler, consumer_handler, producer_handler

try:
    import websockets
except:
    raise Exception("The server command requires the websockets library to be installed")


async def producer(self, websocket, path):
    pass

async def consumer(self, websocket, path):
    pass

def start(ctx):

    connected = set()

    wrapped_producer = functools.partial(producer_handler, producer)
    wrapped_consumer = functools.partial(consumer_handler, consumer)

    wrapped_handler = functools.partial(handler, connected, wrapped_producer, wrapped_consumer)

    asyncio.get_event_loop().run_until_complete(wrapped_handler())

