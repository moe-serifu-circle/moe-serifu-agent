#!/usr/bin/env python
# -*- coding: utf-8 -*-

from schema import Schema
from msa.builtins.conversation import handlers

# from msa.builtins.conversation import entities
from msa.builtins.conversation import server_api
from msa.builtins.conversation import client_api

handler_factories = [handlers.ConversationInputEventHandler]

entities_list = []

register_client_api = client_api.register_endpoints
register_server_api = server_api.register_routes


config_schema = Schema(None)
