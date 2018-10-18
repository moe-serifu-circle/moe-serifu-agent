import sys

import click
import asyncio

import supervisor


@click.command()
@click.option("--cfg", default="msa.cfg", help="The config file to use")
@click.option('--debug', is_flag=True, help="A flag to enable debug logging")
def main(cfg, debug):

    if debug:
        print(f"Using config file '{cfg}'")

    # perform msa::init() equivalent here
    supervisor.init()

    # perform msa::start() equivalent here
    supervisor.start()


if __name__ == "__main__":
    main()
