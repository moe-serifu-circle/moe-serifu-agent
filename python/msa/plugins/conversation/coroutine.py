import asyncio

from msa.coroutine import Coroutine, reschedule
from msa.prompt import Prompt


class KeyboardInputCoroutine(Coroutine):

    def __init__(self):
        super().__init__()
        self.prompt = Prompt()

        self.printed_prompt = False

    @reschedule
    async def work(self, *args):
        msg = await self.prompt("What would you like to eat?:\n1)Pears\n2)Apples\n> ", end="", wait=True)

        msg = msg.lower()

        if msg == "pears":
            print("Yummy, yummy pears!")

        elif msg == "apples":
            print("Keeping the doctor away and all that :D")

        elif msg == "quit":
            print("Well if you insist... Bye, bye!")
            from msa.supervisor import stop
            stop()
            return

        else:
            print(f"Well I don't know what {msg} is but it sounds like you enjoy it!")

        print() # add an extra line for visibility

        await asyncio.sleep(0.5)
