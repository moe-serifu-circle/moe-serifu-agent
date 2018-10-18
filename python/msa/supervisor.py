import asyncio
import sys
from contextlib import suppress


from coroutine import HelloWorldCoroutine, KeyboardInputCoroutine


loop = asyncio.get_event_loop()

coroutines = []
event_queue = asyncio.Queue()
stop_loop = False

def init():

    coroutines.append(KeyboardInputCoroutine())
    coroutines.append(HelloWorldCoroutine())



async def main_coro():
    #for coro in coroutines:
    #    # "paralellizes" tasks, scheduling them on the event loop
    #    asyncio.ensure_future(coro.work(), loop=loop)

    futures = asyncio.gather(*[coro.work() for coro in coroutines], return_exceptions=True)

    while not stop_loop:
        await asyncio.sleep(0.5)

    with suppress(asyncio.CancelledError):
        await futures


def start():

    try:
        with suppress(asyncio.CancelledError):
            loop.run_until_complete(main_coro())
    finally:
        loop.close()

def stop():

    asyncio.ensure_future(exit())



async def exit():
    global stop_loop
    stop_loop = True

    loop = asyncio.get_event_loop()

    pending = asyncio.Task.all_tasks()
    pending.remove(asyncio.Task.current_task())
    for task in pending:
        with suppress(asyncio.CancelledError):
            task.cancel()
            await task



