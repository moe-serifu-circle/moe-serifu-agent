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

        register_event = RegisterCommandEvent(factory)

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
            print("Ok, shutting down...")
            stop()

        await asyncio.sleep(0.1)


    def register_factory(self, event):
        new_factory = event.data["factory"]

        # verify that no other commands utilize the same invoke keyword
        for event_factory in self.event_factories:
            if event_factory.invoke.lower() == new_factory.invoke.lower():
                print(f"Command with invoke keyword '{new_factory.invoke}' already defined")

        self.event_factories.append(new_factory)


    def display_help(self, event):

        command = event.data["command"]

        if command is None:
            # print availiable commands

            print("Availiable commands:")
            for event_factory in self.event_factories:
                print(f"{event_factory.invoke}: {event_factory.describe}")
            print()


        else:
            for event_factory in self.event_factories:
                if command == event_factory.invoke:
                    print(f"Help text for command '{command}':\n{event_factory.describe}\n{event_factory.usage}\n")




