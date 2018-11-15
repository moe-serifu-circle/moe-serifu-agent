from msa.builtins.command_registry.events import CommandEvent


class EchoCommandEvent(CommandEvent):
    """A command event handled by EchoHandler, containing the message to be echoed"""
    def __init__(self):
        super().__init__(priority=10)

