import asyncio
import sys

from msa import supervisor
from msa.coroutine import Coroutine, reschedule

from msa.builtins.terminal_input.event import TextInputEvent
from msa.builtins.command.event import CommandEventFactory, RegisterCommandEvent

from msa.plugins.conversation.event import ConverseCommandEvent

class ConversationCoroutine(Coroutine):

    def __init__(self):
        super().__init__()

        self.prompted = False


    async def init(self):

        factory = CommandEventFactory(
                event_constructor=ConverseCommandEvent,
                invoke="conv",
                describe="Have a nice conversation with your AI.",
                usage="$conv [response]\ne.g. $conv apple"
                )

        register_event = RegisterCommandEvent(factory)

        await supervisor.propogate_event(register_event)



    @reschedule
    async def work(self, event_queue):
        if not self.prompted:
            print("What would you like to eat?\n1)Apples\n2)Pears")
            self.prompted = True

        # get event
        event = await event_queue.get()
        event_queue.task_done()

        if not event.propogate:
            await asyncio.sleep(0.1)
            return

        if not isinstance(event, ConverseCommandEvent):
            await asyncio.sleep(0.1)
            return

        self.prompted = False

        tokens = event.data["tokens"][1: len(event.data["tokens"])]
        msg = ''.join(tokens)

        msg = msg.lower()

        if msg == "pears":
            print("Yummy, yummy pears!")

        elif msg == "apples":
            print("Keeping the doctor away and all that :D")

        elif msg == "quit":
            print("Well if you insist... Bye, bye!", flush=True)
            from msa.supervisor import stop
            stop()
            return

        else:
            print(f"Well I don't know what {msg} is but it sounds like you enjoy it!")

        print() # add an extra line for visibility

        await asyncio.sleep(0.5)
