import functools
import asyncio

from msa import supervisor
from msa.modes import Modes

from msa.remote.generic_producer_consumer import *
try:
    import websockets
except:
    raise Exception("The server command requires the websockets library to be installed")


event_queue = supervisor.create_event_queue()

async def producer():

    while True:
        global event_queue
        event = await event_queue.get()
        #print("server sending event", event)

        if event != None and event.propogateRemote:
            serialized_event = serialize_event(event)
            if serialized_event is not None:
                break # we have a valid event to propogate, prevents events from going back and fourth

    return serialized_event

async def consumer(message):
    #print("server recieving event", message)

    new_event = deserialize_event(message)

    if new_event is not None:
        await supervisor.propogate_event(new_event)


def start(ctx):

    connected = set()

    wrapped_producer = functools.partial(producer_handler, producer)
    wrapped_consumer = functools.partial(consumer_handler, consumer)

    wrapped_handler = functools.partial(handler, connected, wrapped_producer, wrapped_consumer)


    async def start_server():
        ws_server = await websockets.serve(wrapped_handler, '127.0.0.1', 8765)

        cb = functools.partial(server_shutdown_handler, ws_server)
        supervisor.register_shutdown_handler(cb)

    supervisor.init(Modes.server)
    supervisor.start(additional_coros=[start_server()])


