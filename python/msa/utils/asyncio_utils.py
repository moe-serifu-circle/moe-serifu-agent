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
