#!/usr/bin/env python
# -*- coding: utf-8 -*-


class MsaApi(dict):
    def __init__(self, *args, **kwargs):
        super(MsaApi, self).__init__(*args, **kwargs)
        self.__dict__ = self
