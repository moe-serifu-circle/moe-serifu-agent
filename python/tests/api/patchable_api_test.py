#!/usr/bin/env python
# -*- coding: utf-8 -*-


import unittest

from msa.api.patchable_api import MsaApi


class PatchableApiTest(unittest.TestCase):
    def test_str(self):
        api = MsaApi()
        api_str = str(api)

        assert MsaApi.__name__ in api_str

    def test_patching_methods_onto_api(self):
        def new_method():
            return "patched_method_result"

        api = MsaApi()

        api.new_method = new_method

        assert getattr(api, "new_method", None) is not None
        assert callable(api.new_method)

        assert api.new_method() == "patched_method_result"
