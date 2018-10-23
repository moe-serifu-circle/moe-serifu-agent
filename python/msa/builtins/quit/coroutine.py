import asyncio
import sys

from msa import supervisor
from msa.coroutine import Coroutine, reschedule

from msa.builtins.command.event import RegisterCommandEvent, CommandEventFactory
from msa.builtins.quit.event import QuitCommandEvent

class QuitCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

        self.event_factories = []


    async def init(self):

        factory = CommandEventFactory(
                QuitCommandEvent,
                invoke="quit",
                describe="Exits the msa.",
                usage="$quit")

        register_event = RegisterCommandEvent()
        register_event.init(factory)

        await supervisor.propogate_event(register_event)



    @reschedule
    async def work(self, event_queue):


        event = await event_queue.get()
        event_queue.task_done()

        if not event.propogate:
            await asyncio.sleep(0.01)
            return

        if isinstance(event, QuitCommandEvent):
            from msa.supervisor import stop
            print("Ok, shutting down...") # leave as print as events will be ignored after stop
            stop()

        await asyncio.sleep(0.1)


