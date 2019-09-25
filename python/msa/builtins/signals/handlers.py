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

    async def init(self):
        # trigger startup hook later
        self.loop.call_later(1, self.trigger_event)
        

    def trigger_event(self):
        new_event = events.StartupEvent()
        new_event.init({
            "timestamp": datetime.datetime.now().strftime("%Y-%m-%d, %H:%M:%S:%f")
        })
        supervisor.fire_event(new_event)


