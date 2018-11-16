from msa.builtins.command_registry.events import CommandEvent


class QuitCommandEvent(CommandEvent):
    """A command event handled by QuitHandler, shuts down msa"""
    def __init__(self):
        super().__init__(priority=0)

