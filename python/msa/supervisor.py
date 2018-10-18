import asyncio
import sys
from contextlib import suppress
import importlib


from msa.coroutine import HelloWorldCoroutine


loop = asyncio.get_event_loop()

registered_coroutines = []
event_queue = asyncio.Queue()
stop_loop = False
stop_main_coro = False
stop_future = None

def init():

    plugins = [
        "conversation"
    ]

    plugin_modules = []

    # load modules
    for module_name in plugins:
        module = importlib.import_module("msa.plugins." + module_name + ".module")
        plugin_modules.append(module.PluginModule)


    # register coroutines
    for module in plugin_modules:
        for coro in module.coroutines:
            registered_coroutines.append(coro)


    registered_coroutines.append(HelloWorldCoroutine())




async def main_coro():
    # "paralellizes" tasks, scheduling them on the event loop

    primed_coroutines = [coro.work() for coro in registered_coroutines]
    futures = asyncio.gather(*primed_coroutines, return_exceptions=True)

    while not stop_main_coro:
        await asyncio.sleep(0.5)

    with suppress(asyncio.CancelledError):
        await futures

    # cancel and suppress exit future
    if stop_future is not None:
        with suppress(asyncio.CancelledError):
            stop_future.cancel()
            await stop_future


def start():

    try:
        with suppress(asyncio.CancelledError):
            loop.run_until_complete(main_coro())
    finally:
        loop.close()

def stop():
    global stop_future
    stop_future = asyncio.ensure_future(exit())


async def exit():
    global stop_loop
    stop_loop = True

    await asyncio.sleep(1)

    stop_main_coro = True

    loop = asyncio.get_event_loop()

    pending = asyncio.Task.all_tasks()
    pending.remove(asyncio.Task.current_task())

    for task in pending:
        with suppress(asyncio.CancelledError):
            task.cancel()
            await task



