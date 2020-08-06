from typing import TYPE_CHECKING

supervisor_instance = None


def get_supervisor() -> "Supervisor":
    if not supervisor_instance:
        raise Exception("Supervisor has not yet been set up!")
    return supervisor_instance
