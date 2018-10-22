import asyncio
import sys

from msa import supervisor
from msa.coroutine import Coroutine, reschedule

from msa.builtins.command.event import RegisterCommandEvent, CommandEventFactory
from msa.builtins.help.event import HelpCommandEvent
from msa.builtins.print.event import PrintTextEvent

class HelpCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

        self.event_factories = []


    async def init(self):

        factory = CommandEventFactory(
                HelpCommandEvent,
                invoke="help",
                describe="Display help for a given command",
                usage="$help [command]\ne.g. $help help")

        register_event = RegisterCommandEvent(factory)

        await supervisor.propogate_event(register_event)



    @reschedule
    async def work(self, event_queue):


        event = await event_queue.get()
        event_queue.task_done()


        if not event.propogate:
            await asyncio.sleep(0.01)
            return

        if isinstance(event, RegisterCommandEvent):
            await self.register_factory(event)
        elif isinstance(event, HelpCommandEvent):
            await self.display_help(event)


        await asyncio.sleep(0.1)


    async def register_factory(self, event):
        new_factory = event.data["factory"]

        # verify that no other commands utilize the same invoke keyword
        for event_factory in self.event_factories:
            if event_factory.invoke.lower() == new_factory.invoke.lower():
                print_event = PrintTextEvent(f"Command with invoke keyword '{new_factory.invoke}' already defined")
                await supervisor.propogate_event(print_event)

        self.event_factories.append(new_factory)


    async def display_help(self, event):

        command = event.data["command"]

        if command is None:
            # print availiable commands


            out = "Availiable commands:\n"
            for event_factory in self.event_factories:
                out += f"{event_factory.invoke}: {event_factory.describe}"
            out += "\n"

            print_event = PrintTextEvent(out)
            await supervisor.propogate_event(print_event)

        else:
            for event_factory in self.event_factories:
                if command == event_factory.invoke:
                    out = f"Help text for command '{command}':\n{event_factory.describe}\n{event_factory.usage}\n"
                    print_event = PrintTextEvent(out)
                    await supervisor.propogate_event(print_event)




