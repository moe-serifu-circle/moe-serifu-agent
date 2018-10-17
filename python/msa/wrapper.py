import asyncio

def reschedule(fn, args=[]):
    async def wrapped_cb(self, args=[]):

        result = await fn(self, *args)
        loop = asyncio.get_event_loop()
        asyncio.ensure_future(self.handle(), loop=loop)

        return result

    return wrapped_cb

