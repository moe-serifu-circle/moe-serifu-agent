import logging
import click

from msa.core import supervisor, RunMode

@click.group(invoke_without_command=True)
@click.option("--cfg", default="msa.cfg", help="The config file to use")
@click.option('--log-level', default="INFO", help="Override the log level defined in the config file. Can be either: "
                                                  "'debug', 'info' 'warn', or 'error'")
@click.pass_context
def main(ctx, cfg, log_level):

    ctx.obj["log_level"] = getattr(logging, log_level.upper(), logging.INFO)
    ctx.obj["cfg"] = cfg

    if ctx.invoked_subcommand is None:
        ctx.invoke(cli)

@main.command()
@click.pass_context
def cli(ctx):

    supervisor.init(RunMode.CLI, ctx.obj)

    supervisor.start()


if __name__ == "__main__":
    main(obj={})
