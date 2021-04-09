#!/usr/bin/env python
# -*- coding: utf-8 -*-

from schema import Schema
from msa.builtins.intents import handlers


handler_factories = [handlers.IntentToEventHandler]

entities_list = []


config_schema = Schema(None)
