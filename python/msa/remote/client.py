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

    wrapped_producer = functools.partial(producer_handler, producer, None)
    wrapped_consumer = functools.partial(consumer_handler, consumer)

    wrapped_handler = functools.partial(handler, connected, wrapped_producer, wrapped_consumer)


    async def start_client(host, port):
        async with websockets.connect(
                f'ws://{host}:{port}') as websocket:

            # register shudown handler
            cb = functools.partial(client_shutdown_handler, websocket)
            supervisor.register_shutdown_handler(cb)

            await wrapped_handler(websocket, "")


    if ctx.obj["debug"]:
        print(f"Starting client on ws://{ctx.obj['host']}:{ctx.obj['port']}")

    start_client = functools.partial(start_client, ctx.obj["host"], ctx.obj["port"])

    supervisor.init(Modes.client)
    supervisor.start(additional_coros=[start_client()])


