import sys
import asyncio


class Prompt:
    def __init__(self, loop=None):
        self.loop = loop or asyncio.get_event_loop()
        self.q = asyncio.Queue(loop = self.loop)
        self.loop.add_reader(sys.stdin, self.handle_input)

    def handle_input(self):
        future = self.q.put(sys.stdin.readline())
        asyncio.ensure_future(future, loop=self.loop)

    async def __call__(self, msg, end="\n", flush=False, wait=False):
        print(msg, end=end, flush=flush)

        if wait:
            return (await self.q.get()).rstrip("\n")
        else:
            try:
                return self.q.get_nowait()
            except asyncio.QueueEmpty:
                return None


