import asyncio
import sys
from contextlib import suppress
import importlib


from msa.coroutine import HelloWorldCoroutine


loop = asyncio.get_event_loop()

event_queue = asyncio.Queue()
stop_loop = False
stop_main_coro = False
stop_future = None

registered_coroutines = []
event_queues = {}
registered_event_types = {}
propogation_hooks = []

def init():
    builtins = [
        "terminal_input",
        "command",
        "help",
        "quit",
        "print",
    ]

    plugins = [
        "conversation",
    ]

    plugin_modules = []

    # load built in modules
    for module_name in builtins:
        module = importlib.import_module("msa.builtins." + module_name + ".module")
        plugin_modules.append(module.PluginModule)

    # load plugin modules
    for module_name in plugins:
        module = importlib.import_module("msa.plugins." + module_name + ".module")
        plugin_modules.append(module.PluginModule)


    # register coroutines
    for module in plugin_modules:
        for coro in module.coroutines:
            registered_coroutines.append({
                "coroutine": coro,
                "event_queue": asyncio.Queue()
            })

    # register event types
    for module in plugin_modules:
        global registered_event_types
        registered_event_types = {**registered_event_types, **module.events}


    registered_coroutines.append({
        "coroutine": HelloWorldCoroutine(),
        "event_queue": asyncio.Queue()
    })



async def propogate_event(new_event):
    global registered_coroutines

    for coro in registered_coroutines:
        coro["event_queue"].put_nowait(new_event)

    for hook in propogation_hooks:
        hook.put_nowait(new_event)



async def main_coro(additional_coros=[]):
    # "paralellizes" tasks, scheduling them on the event loop

    init_coroutines = [coro["coroutine"].init() for coro in registered_coroutines]
    await asyncio.gather(*init_coroutines, return_exceptions=True)

    primed_coroutines = [coro["coroutine"].work(coro["event_queue"]) for coro in registered_coroutines]
    if len(additional_coros) > 0:
        primed_coroutines.extend(additional_coros)
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

def create_event_queue():
    new_queue = asyncio.Queue()

    global propogation_hooks
    propogation_hooks.append(new_queue)

    return new_queue

def start(additional_coros=[]):

    try:
        with suppress(asyncio.CancelledError):
            primed_coro = main_coro(additional_coros)
            loop.run_until_complete(primed_coro)
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



