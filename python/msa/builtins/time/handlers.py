import asyncio
from datetime import datetime, timedelta

from msa.core.event_handler import EventHandler
from msa.core import supervisor

from msa.builtins.time.events import TimeEvent


class TimeHandler(EventHandler):
    """
    Fires a TimeEvent at the beginning of every minute
    """

    def __init__(self, loop, event_queue, logger, config=None):
        super().__init__(loop, event_queue, logger, config)

        self.last_tick = datetime.now()


    async def handle(self):


        # calculate time till next minute
        now = datetime.now()
        next_tick = now.replace(microsecond=0, second=0) + timedelta(minutes=1, seconds=1)
        delta = next_tick - now

        if delta.seconds > 0:
            await asyncio.sleep(delta.seconds)

        self.last_tick = datetime.now()

        new_event = TimeEvent()
        new_event.init({
            "current_time": self.last_tick
        })

        supervisor.fire_event(new_event)

