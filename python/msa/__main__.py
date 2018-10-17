import sys

import click
import asyncio

from wrapper import reschedule
from prompt import Prompt
from coroutine import HelloWorldCoroutine, KeyboardInputCoroutine


loop = asyncio.get_event_loop()


coroutines = []

def init():

    coroutines.append(KeyboardInputCoroutine())
    coroutines.append(HelloWorldCoroutine())



def main_coro():
    for coro in coroutines:
        asyncio.ensure_future(coro.work(), loop=loop)

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
