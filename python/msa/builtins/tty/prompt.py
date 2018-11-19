import sys
import asyncio
import platform

from msa.core import supervisor


class Prompt:
    """An asyncio based text prompt. Listens for input from a TTY. Used by the TtyInputHandler to get input."""
    def __init__(self, loop):
        self.loop = loop
        self.q = asyncio.Queue(loop = self.loop)

        if platform.system() != "Windows":
            self.loop.add_reader(sys.stdin, self.handle_input)

    def handle_input(self):
        """Callback used by the asyncio loop reader when there is input to stdin"""
        future = self.q.put(sys.stdin.readline())
        asyncio.ensure_future(future, loop=self.loop)

    def listen(self, wait=False):
        """Listen for prompt input"""

        if platform.system() == "Windows":
            return asyncio.get_event_loop().run_in_executor(supervisor.executor, sys.stdin.readline)

        # if mac or linux
        if wait:
            return self.q.get()
        else:
            try:
                return self.q.get_nowait()
            except asyncio.QueueEmpty:
                return None

