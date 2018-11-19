
import click

from msa.core import supervisor, RunMode

@click.group(invoke_without_command=True)
@click.option("--cfg", default="msa.cfg", help="The config file to use")
@click.option('--debug', is_flag=True, help="A flag to enable debug logging")
@click.pass_context
def main(ctx, cfg, debug):

    if debug:
        print(f"Using config file '{cfg}'")

    ctx.obj["debug"] = debug
    ctx.obj["cfg"] = cfg

    if ctx.invoked_subcommand is None:
        ctx.invoke(cli)

@main.command()
@click.pass_context
def cli(ctx):

    supervisor.init(RunMode.CLI)

    supervisor.start()


if __name__ == "__main__":
    main(obj={})
