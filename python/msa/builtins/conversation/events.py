#!/usr/bin/env python
# -*- coding: utf-8 -*-
from schema import Schema, And, Or, Optional

from msa.core.event import Event


class ConversationInputEvent(Event):
    """
    ConversationInputEvent schema:
    - input: A conversational statement or question to the agent.
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema({"input": And(str, len)}))


class ConversationOutputEvent(Event):
    """
    ConversationOutputEvent schema:
    - output: A conversational response to a statement or question given to the agent.
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema({"output": And(str, len)}))
