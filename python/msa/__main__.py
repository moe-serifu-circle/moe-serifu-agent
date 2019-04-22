import logging
import click

from msa.core import supervisor, RunMode

@click.group(invoke_without_command=True)
@click.option("--config-file", default="msa_config.json", help="The config file to use")
@click.option('--log-level', help="Override the log level defined in the config file. Can be either: "
                                                  "'debug', 'info' 'warn', or 'error'")
@click.pass_context
def main(ctx, config_file, log_level):

    ctx.obj["log_level"] = log_level

    ctx.obj["config_file"] = config_file

    if ctx.invoked_subcommand is None:
        ctx.invoke(daemon)

@main.command()
@click.pass_context
def daemon(ctx):

    supervisor.init(RunMode.CLI, ctx.obj)

    supervisor.start()

@main.command()
@click.pass_context
def cli(ctx):
    from msa.cli.interpreter import Interpreter

    interpreter = Interpreter()
    interpreter.start()




if __name__ == "__main__":
    main(obj={})
