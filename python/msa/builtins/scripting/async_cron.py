import time
import asyncio
from datetime import datetime
from tzlocal import get_localzone
from croniter import croniter
from itertools import chain


class AsyncCrontab:
    def __init__(self, crontab, loop):
        self.crontab = crontab
        self.loop = loop
        self.tz = get_localzone()
        self.base = datetime.now(self.tz)
        self.time = time.time()
        self.loop_time = loop.time()
        self.croniter = croniter(crontab, start_time=self.base).all_next(datetime)
        self.iter = self.croniter
        self.scheduled_time = self.peek()


    def peek(self):
        """
        Chech the 
        """
        next_element = next(self.iter)
        self.iter = chain([next_element], self.iter)
        return next_element

    def current(self):
        return self.scheduled_time

    async def next(self):
        self.scheduled_time = next(self.iter)
        ts = self.scheduled_time.timestamp()
        relative_time = self.loop_time + (ts - self.time)
        await asyncio.sleep(relative_time)




