import unittest
from unittest.mock import patch, MagicMock


from msa.core.loader import load_builtin_modules, load_plugin_modules, builtin_module_names

class LoaderTest(unittest.TestCase):

    @patch("importlib.import_module")
    def test_loading_builtin_modules(self, import_module_mock):
        module_source = {
            f"msa.builtins.{name}": FakeModule() for name in builtin_module_names
        }

        import_module_mock.side_effect = lambda e: module_source[e]

        modules = load_builtin_modules()

        self.assertEqual(
            builtin_module_names,
            [m.module_name for m in modules]
        )


    @patch("importlib.import_module")
    def test_loading_plugin_modules(self, import_module_mock):

        module_names = [
            "test_module_1",
            "test_module_2",
            "test_module_3"
        ]
        module_source = {
            f"msa.plugins.{name}": FakeModule() for name in module_names
        }

        import_module_mock.side_effect = lambda e: module_source[e]

        modules = load_plugin_modules(module_names)

        self.assertEqual(
            module_names,
            [m.module_name for m in modules]
        )




class FakeModule:
    pass
