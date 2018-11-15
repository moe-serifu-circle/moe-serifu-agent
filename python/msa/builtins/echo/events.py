from msa.builtins.command_registry.events import CommandEvent


class EchoCommandEvent(CommandEvent):
    def __init__(self):
        super().__init__(priority=10)

