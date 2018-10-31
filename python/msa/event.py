from enum import Enum


class Topics(Enum):
    # Greater priority topics interrupt event handling of lower-numbered priority topics.
    EVENT_STACK_CLEARED = 0
    EVENT_HANDLED = 0
    EVENT_INTERRUPTED = 0
    TEXT_INPUT = 10
