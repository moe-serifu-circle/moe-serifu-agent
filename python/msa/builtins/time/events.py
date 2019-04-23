import datetime

from schema import Schema

from msa.core.event import Event
from msa.builtins.command_registry.events import CommandEvent


class TimeEvent(Event):

    def __init__(self):
        super().__init__(
            priority=100,
            schema=Schema({
                "current_time": float
            })
        )


class DelTimerCommandEvent(CommandEvent):
    """
    Event corresponding to the deltimer command being entered.
    """
    def __init__(self):
        super().__init__(priority=10)


class TimerCommandEvent(CommandEvent):
    """
    Event corresponding to the timer command being entered.
    """
    def __init__(self):
        super().__init__(priority=10)

