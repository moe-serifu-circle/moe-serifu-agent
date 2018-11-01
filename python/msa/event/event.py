from enum import Enum, auto


class Topics(Enum):
    STACK_CLEARED = (auto(), 0)
    HANDLED = (auto(), 0)
    INTERRUPTED = (auto(), 0)
    TEXT_INPUT = (auto(), 10)

    def __init__(self, id, piority):
        self._id = id
        self._priority = piority

    @property
    def priority(self):
        return self._priority
