import functools
import asyncio

from msa.remote.generic_producer_consumer import *
from msa import supervisor
from msa.modes import Modes

try:
    import websockets
except:
    raise Exception("The server command requires the websockets library to be installed")


event_queue = supervisor.create_event_queue()

async def producer():

    while True:
        global event_queue
        event = await event_queue.get()
        #print("client sending event", event)

        if event != None and event.propogateRemote:
            serialized_event = serialize_event(event)
            if serialized_event is not None:
                break # we have a valid event to propogate, prevents events from going back and fourth

    return serialized_event

async def consumer(message):
    #print("client recieving event", message)

    new_event = deserialize_event(message)

    if new_event is not None:
        await supervisor.propogate_event(new_event)



def start(ctx):

    connected = set()

    wrapped_producer = functools.partial(producer_handler, producer)
    wrapped_consumer = functools.partial(consumer_handler, consumer)

    wrapped_handler = functools.partial(handler, connected, wrapped_producer, wrapped_consumer)


    async def start_client():
        async with websockets.connect(
                'ws://127.0.0.1:8765') as websocket:

            # register shudown handler
            cb = functools.partial(client_shutdown_handler, websocket)
            supervisor.register_shutdown_handler(cb)

            await wrapped_handler(websocket, "")

    supervisor.init(Modes.client)
    supervisor.start(additional_coros=[start_client()])

