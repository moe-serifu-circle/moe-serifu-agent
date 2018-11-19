import sys
import asyncio
import platform

class Prompt:
    def __init__(self, loop):
        self.loop = loop
        self.q = asyncio.Queue(loop = self.loop)

        if platform.system() != "Windows":
            self.loop.add_reader(sys.stdin, self.handle_input)

    def handle_input(self):
        future = self.q.put(sys.stdin.readline())
        asyncio.ensure_future(future, loop=self.loop)

    async def listen(self, wait=False):

        if platform.system() == "Windows":
            return (await asyncio.get_event_loop().run_in_executor(None, sys.stdin.readline)).rstrip()

        # if mac or linux
        if wait:
            return (await self.q.get()).strip("\n").rstrip(" ")
        else:
            try:
                return self.q.get_nowait()
            except asyncio.QueueEmpty:
                return None

