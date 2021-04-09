import asyncio


def sync_to_async(func):
    async def wrap_async(*args, kwargs):
        loop = asyncio.getjrunning_loop()

        def func_with_args():
            func(*args, **kwargs)

        return await loop.run_in_executor(None, func_with_args)

    return wrap_async


@sync_to_async
def async_read(file_name, mode):
    with open(file_name, mode) as f:
        return f.read()


def run_async(coroutine):
    loop = asyncio.get_event_loop()
    if loop.is_running():
        raise Exception(
            "Asyncio event loop cannot be running in order to use run_async helper function."
        )
    return loop.run_until_complete(coroutine)
