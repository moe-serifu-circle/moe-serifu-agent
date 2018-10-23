import asyncio
import sys

from msa import supervisor
from msa.coroutine import Coroutine, reschedule

from msa.builtins.terminal_input.event import TextInputEvent
from msa.builtins.command.event import RegisterCommandEvent

class CommandCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

        self.event_factories = []


    @reschedule
    async def work(self, event_queue):


        event = await event_queue.get()
        event_queue.task_done()


        if not event.propogate:
            await asyncio.sleep(0.01)
            return

        if isinstance(event, RegisterCommandEvent):
            self.register_factory(event)

        elif isinstance(event, TextInputEvent):
            await self.parse_text_input(event.data["text"])

        await asyncio.sleep(0.1)


    def register_factory(self, event):
        new_factory = event.data["factory"]

        # verify that no other commands utilize the same invoke keyword
        for event_factory in self.event_factories:
            if event_factory.invoke.lower() == new_factory.invoke.lower():
                print(f"Command with invoke keyword '{new_factory.invoke}' already defined")

        self.event_factories.append(new_factory)


    async def parse_text_input(self, raw_text):
        # tokenize
        if len(raw_text) == 0:
            return

        tokens = raw_text.split()

        invoke_keyword = tokens[0]

        # search event factories for one that corresponds to invoke_keyword
        for event_factory in self.event_factories:
            if invoke_keyword == event_factory.invoke:


                new_event = event_factory.create_event({
                        "raw_text_input": raw_text,
                        "tokens": tokens
                    })

                # found appropriate event type
                await supervisor.propogate_event(new_event)

                break # break out as we are done here



