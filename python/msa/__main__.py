import sys

import click
import asyncio

from wrapper import reschedule
from prompt import Prompt

loop = asyncio.get_event_loop()

handlers = []


def init():

    # init all handlers
    # in practice these should all be methods on classes
    class a:
        def __init__(self):
            self.prompt = Prompt()

        @reschedule
        async def handle(self, *args):

            msg = await self.prompt("prompt: ", wait=True)
            print(msg)

            await asyncio.sleep(0.5)

    class b:
        @reschedule
        async def handle(self, *args):
            print("hello world 2")
            sys.stdout.flush()

            await asyncio.sleep(3)


    _a = a()
    _b = b()


    handlers.append(_b)
    handlers.append(_a)


def main_coro():
    for obj in handlers:
        asyncio.ensure_future(obj.handle(), loop=loop)

def start():
    main_coro()
    loop.run_forever()

def quit():
    pass


@click.command()
@click.option("--cfg", default="msa.cfg", help="The config file to use")
@click.option('--debug', is_flag=True, help="A flag to enable debug logging")
def main(cfg, debug):

    if debug:
        print(f"Using config file '{cfg}'")

    # perform msa::init() equivalent here
    init()

    # perform msa::start() equivalent here
    start()

    # perform msa::quit() equivalent here


if __name__ == "__main__":
    main()
