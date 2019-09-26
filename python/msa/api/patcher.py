#!/usr/bin/env python
# -*- coding: utf-8 -*-
from functools import partial

from msa.api.patchable_api import MsaApi

from msa.api.base_methods import register_base_methods
from msa.core.loader import load_builtin_modules, load_plugin_modules

class ApiPatcher:
    cache = {}
    
    def __init__(self, api_context, api_client):
        self.api = MsaApi()
        self.api.context = api_context
        self.api.client = api_client

        self._registration_frozen = False
        self.process_registration()
        self._registration_frozen = True

    @staticmethod
    def load(self, api_context, api_client=None):
        if api_context is None:
            raise Exception(f"{ApiPatcher.__name__}: api_context cannot be None.")

        if api_context in ApiPatcher.cache:
            if api_client is not None:
                raise Exception(f"{ApiPatcher.__name__}: api_client cannot be non-None when context '{api_context}' already been patched and loaded.")
            
            return ApiPatcher.cache[api_context]

        else:
            if api_client is None:
                raise Exception(f"{ApiPatcher.__name__}: api_client cannot be None when context '{api_context}' has never been patched and loaded.")

            ApiPatcher.cache[api_context] = ApiPatcher(api_context, api_client)
            return ApiPatcher.cache[api_context]
            
    def _process_registrations(self):
        register_base_methods(self)

        for module in load_builtin_modules():
            if hasattr(module, "register_client_api") and callable(module.register_client_api):
                module.register_client_api(self)

        for module in load_plugin_modules(white_listed_plugins):
            if hasattr(module, "register_client_api") and callable(module.register_client_api):
                module.register_client_api(self)


    def register_method(self):
        if not self._registration_frozen:
            def decorator(func):
                self.api[func.__name__] = partial(func, self.api)
            return decorator
        else:
            raise Exception(f"MsaAPI Method Registration is frozen, failed to register method: {func.__name__}")

