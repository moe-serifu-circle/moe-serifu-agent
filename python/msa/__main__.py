import os
import logging
import click

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

    from msa.server import start_server
    start_server(ctx.obj)


@main.command()
@click.option("-s" , "--script",  help="If provided will run the specified script and then exit.")
@click.pass_context
def cli(ctx, script):
    from msa.cli.interpreter import Interpreter

    interpreter = Interpreter()

    if script is not None:
        if os.path.exists(script) and os.path.isfile(script):
            interpreter.execute_script(script)
        else:
            print("-s flag provided. Unable to load script {}. Please verify that the file exists and the provided path is correct.".format(script))
    else:
        interpreter.start()


if __name__ == "__main__":
    main(obj={})
