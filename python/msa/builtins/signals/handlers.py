import datetime 
from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.signals import events

class StartupEventTrigger(EventHandler):
    """
    Fires off a startup event and then exits
    """

    def __init__(self, loop, event_bus, database, logger, config=None):
        super().__init__(loop, event_bus, database, logger, config)

    async def handle(self):
        # prevent handler from being rescheduled

        new_event = events.StartupEvent()
        new_event.init({
            "timestamp": datetime.datetime.now().strftime("%Y-%m-%d, %H:%M:%S:%f")
        })
        await self.event_bus.fire_event(new_event)

        self.cancel_reschedule()

