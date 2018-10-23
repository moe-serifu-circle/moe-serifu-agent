import click

from msa import supervisor
from msa.modes import Modes


@click.group(invoke_without_command=True)
@click.option("--cfg", default="msa.cfg", help="The obj file to use")
@click.option('--debug', is_flag=True, help="A flag to enable debug logging")
@click.pass_context
def main(ctx, cfg, debug):

    if debug:
        print(f"Using obj file '{cfg}'")

    ctx.obj["debug"] = debug
    ctx.obj["cfg"] = cfg

    if ctx.invoked_subcommand is None:
        cli()


@main.command()
@click.pass_context
def cli(ctx):

    # perform msa::init() equivalent here
    supervisor.init()

    # perform msa::start() equivalent here
    supervisor.start(Modes.cli)

@main.command()
@click.pass_context
def server(ctx):
    from msa.remote import server
    server.start(ctx)

@main.command()
@click.pass_context
def client(ctx):
    from msa.remote import client
    client.start(ctx)


if __name__ == "__main__":
    main(obj={})
