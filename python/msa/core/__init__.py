from msa.core.supervisor import Supervisor

supervisor = Supervisor()

class RunMode:
    CLI = 0
    SERVER = 1
    CLIENT = 2


class LogicError(Exception):
    """
    Raised when there is a problem with a command.
    """

    def __init__(self, msg: str):
        super().__init__(msg)


class ProtectionError(Exception):
    """
    Raised when a privileged operation is attempted in an unprivileged context.
    """

    def __init__(self, msg: str):
        super().__init__(msg)
