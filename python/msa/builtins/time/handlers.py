import asyncio
from datetime import datetime, timedelta
import time
import math

from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.core import timer

from msa.builtins.time.events import TimeEvent


class TimeHandler(EventHandler):
    """
    Fires a TimeEvent at the beginning of every minute
    """

    def __init__(self, loop, event_queue, logger, config=None):
        super().__init__(loop, event_queue, logger, config)

        self.timer_manager = timer.TimerManager()
        # TODO: merge with ctor
        self.timer_manager.tick_resolution = int(config['tick_resolution'])

    async def handle(self):
        # TODO: ensure we aren't skipping timer fire checks too frequently
        start = time.monotonic()  # get now to determine when to wake up

        new_event = TimeEvent()
        new_event.init({
            "current_time": start
        })

        supervisor.fire_event(new_event)

        await self.timer_manager.check_timers()
        # check them again in 50ms - no gaurentee this will happen, but hopefully scheduled frequently enough to not
        # matter. Definitely a monkey patch for now
        wait_time = 0.05
        delta = wait_time - (time.monotonic() - start)
        if delta < 0:
            # we have missed a fire due to firing taking too long. No problem, just skip as many rounds as have been
            # missed

            # calculate wait time until next firing point
            delta = wait_time + math.modf(delta)[0]

        await asyncio.sleep(delta)
