import importlib


# builtin modules to load
builtin_module_names = [
    "time"
]


def load_builtin_modules():
    """Loads builtin modules."""
    plugin_modules = []

    for module_name in builtin_module_names:
        module = importlib.import_module("msa.builtins.{}".format(module_name))
        module.module_name = module_name
        plugin_modules.append(module)

    return plugin_modules


def load_plugin_modules(plugin_module_names, mode):
    """Loads plugin modules as specified in the configuration file."""

    plugin_modules = []

    for module_name in plugin_module_names:
        module = importlib.import_module("msa.plugins.{}".format(module_name))
        module.module_name = module_name

        if mode in module.PluginModule.allowed_modes:
            plugin_modules.append(module)

    return plugin_modules
