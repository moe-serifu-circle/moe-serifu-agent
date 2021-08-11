import unittest
from unittest.mock import patch, MagicMock
import asyncio

from msa.api import get_api
from msa.utils.asyncio_utils import run_async
from msa.api.context import ApiContext
from msa.api import api_clients, patchable_api
from msa.api.patcher import ApiPatcher
from msa.core.loader import builtin_module_names


class GetApiTest(unittest.TestCase):
    def setUp(self):
        # rest cache between tests
        ApiPatcher.cache = {}

    def test_get_api_fake_context(self):

        with self.assertRaises(Exception) as cm:
            get_api("Fake context")

        self.assertEqual(
            "Invalid api context provided: 'Fake context'.", str(cm.exception)
        )

    def test_get_api_no_context(self):

        with self.assertRaises(Exception) as cm:
            get_api(None)

        self.assertEqual("get_api: context cannot be None.", str(cm.exception))

    def test_get_api_local_context_no_args(self):

        with self.assertRaises(Exception) as cm:
            api_instance = get_api(ApiContext.local)

        self.assertEqual(
            "ApiPatcher: api_client cannot be None when context 'ApiContext.local' has never been patched and loaded.",
            str(cm.exception),
        )

    def test_get_api_rest_context_no_args(self):

        with self.assertRaises(Exception) as cm:
            api_instance = get_api(ApiContext.rest)

        self.assertEqual(
            "ApiPatcher: api_client cannot be None when context 'ApiContext.rest' has never been patched and loaded.",
            str(cm.exception),
        )

    def test_get_api_websocket_context_no_args(self):

        with self.assertRaises(Exception) as cm:
            api_instance = get_api(ApiContext.websocket)

        self.assertEqual(
            "ApiPatcher: api_client cannot be None when context 'ApiContext.websocket' has never been patched and loaded.",
            str(cm.exception),
        )

    def test_get_api_local_no_plugin_whitelist(self):
        fake_loop = MagicMock()
        with self.assertRaises(Exception) as cm:
            api_instance = get_api(ApiContext.local, loop=fake_loop)

        self.assertEqual(
            "get_api: plugin_whitelist cannot be none when **kwargs are provided to get_api.",
            str(cm.exception),
        )

    def test_get_api_rest_no_plugin_whitelist(self):
        fake_loop = MagicMock()
        with self.assertRaises(Exception) as cm:
            api_instance = get_api(ApiContext.rest, host="0.0.0.0", port="8080")

        self.assertEqual(
            "get_api: plugin_whitelist cannot be none when **kwargs are provided to get_api.",
            str(cm.exception),
        )

    def test_get_api_websockets_no_plugin_whitelist(self):
        fake_loop = MagicMock()

        async def propagate(self, event_queue):
            event = await event_queue.get()

        async def interact(self):
            pass

        with self.assertRaises(Exception) as cm:
            api_instance = get_api(
                ApiContext.websocket,
                loop=fake_loop,
                interact=interact,
                propagate=propagate,
                host="localhost",
                port="8080",
            )

        self.assertEqual(
            "get_api: plugin_whitelist cannot be none when **kwargs are provided to get_api.",
            str(cm.exception),
        )

    @patch("importlib.import_module")
    def test_get_api_local(self, import_module_mock):
        fake_loop = MagicMock()
        fake_plugins = [
            *[FakeModule() for _ in range(len(builtin_module_names))],
            FakeModule(),
            FakeModule(),
            FakeModule(),
        ]
        import_module_mock.side_effect = fake_plugins
        white_list = ["a", "b", "c"]
        api_instance = get_api(
            ApiContext.local, plugin_whitelist=white_list, loop=fake_loop
        )
        self.assertIsInstance(api_instance, patchable_api.MsaApi)

        self.assertEqual(api_instance.context, ApiContext.local)
        self.assertIsInstance(api_instance.client, api_clients.ApiLocalClient)
        self.assertTrue("ping" in api_instance)

    @patch("importlib.import_module")
    def test_get_api_rest(self, import_module_mock):
        fake_loop = MagicMock()
        fake_plugins = [
            *[FakeModule() for _ in range(len(builtin_module_names))],
            FakeModule(),
            FakeModule(),
            FakeModule(),
        ]
        import_module_mock.side_effect = fake_plugins
        white_list = ["a", "b", "c"]
        api_instance = get_api(
            ApiContext.rest, plugin_whitelist=white_list, host="localhost", port="8080"
        )
        self.assertIsInstance(api_instance, patchable_api.MsaApi)

        self.assertEqual(api_instance.context, ApiContext.rest)
        self.assertIsInstance(api_instance.client, api_clients.ApiRestClient)
        self.assertTrue("ping" in api_instance)

    @patch("importlib.import_module")
    def test_get_api_websockets(self, import_module_mock):
        fake_loop = MagicMock()
        fake_plugins = [
            *[FakeModule() for _ in range(len(builtin_module_names))],
            FakeModule(),
            FakeModule(),
            FakeModule(),
        ]
        import_module_mock.side_effect = fake_plugins
        white_list = ["a", "b", "c"]

        async def propagate(self, event_queue):
            event = await event_queue.get()

        async def interact(self):
            pass

        api_instance = get_api(
            ApiContext.websocket,
            plugin_whitelist=white_list,
            loop=fake_loop,
            interact=interact,
            propagate=propagate,
            host="localhost",
            port="8080",
        )
        self.assertIsInstance(api_instance, patchable_api.MsaApi)

        self.assertEqual(api_instance.context, ApiContext.websocket)
        self.assertIsInstance(api_instance.client, api_clients.ApiWebsocketClient)
        self.assertTrue("ping" in api_instance)


class RunAsyncTest(unittest.TestCase):
    def test_successful_run(self):
        async def test_func():
            return 1 + 1

        result = run_async(test_func())
        self.assertEqual(2, result)

    def test_unsucessful_run(self):
        async def test_wrapper():
            async def test_func():
                return 1 + 1

            run_async(test_func())

        loop = asyncio.get_event_loop()

        with self.assertRaises(Exception) as cm:
            loop.run_until_complete(test_wrapper())

        self.assertEqual(
            "Asyncio event loop cannot be running in order to use run_async helper function.",
            str(cm.exception),
        )


class FakeModule:
    pass
