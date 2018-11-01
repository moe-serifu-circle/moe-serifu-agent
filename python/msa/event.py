from enum import Enum, auto


class Topics(Enum):
    EVENT_STACK_CLEARED = auto()
    EVENT_HANDLED = auto()
    EVENT_INTERRUPTED = auto()
    TEXT_INPUT = auto()

    @property
    def priority(self):
        # Greater priority topics interrupt event handling of lower-numbered priority topics.
        prior_dict = {Topics.EVENT_STACK_CLEARED: 0,
                      Topics.EVENT_HANDLED: 0,
                      Topics.EVENT_INTERRUPTED: 0,
                      Topics.TEXT_INPUT: 10}
        return prior_dict[self]
