#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import asyncio

from msa.api import ApiContext
from msa.api.patcher import ApiPatcher
from msa.api import api_clients

class PatcherTest(unittest.TestCase):

    def setUp(self):
        # clear cache
        ApiPatcher.cache = {}


    def test_init_and_load_with_no_api_context(self):

        with self.assertRaises(Exception) as cm:
            ApiPatcher.load(None)

            self.assertEqual(
                str(cm.exception),
                "ApiPatcher: api_context cannot be None."
            )

    def test_init_and_load_first_time_with_no_api_client(self):

        api_context = ApiContext.local
        
        with self.assertRaises(Exception) as cm:
            ApiPatcher.load(api_context)

        self.assertEqual(str(cm.exception),
                         f"ApiPatcher: api_client cannot be None when context '{api_context}' has never been patched and loaded.")

    def test_init_and_load_first_time_with_api_client(self):

        loop = asyncio.get_event_loop()
        api_context = ApiContext.local
        api_client = api_clients.ApiLocalClient(loop)
        print(api_client)
        
        with self.assertRaises(Exception) as cm:
            ApiPatcher.load(api_context, api_client)

        self.assertEqual(
            str(cm.exception),
            f"ApiPatcher: plugin_whitelist cannot be None when context '{api_context}' has never been patched and loaded."
        )
        
