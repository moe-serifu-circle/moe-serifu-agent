#!/usr/bin/env python
# -*- coding: utf-8 -*-
from enum import Enum


class ApiContext(Enum):
    # use if instead of using a network transport we should call the api implementations
    # directly
    local = 0

    # use for rest API
    rest = 1

    # use for websocket API
    websocket = 2
