import json
import logging
from types import MappingProxyType
from schema import Schema, And, Or, Use

CONFIG_SCHEMA = Schema({
    "agent": {
        "name": And(str, len),
        "user_title": And(str, len)
    },
    "plugin_modules": [
        And(str, len)
    ],

    "module_config": {
        # namespace of module -> dict of config values
    },

    "logging": {
        "global_log_level": And(str,
                                Use(str.upper),
                                lambda s: s in ("DEBUG", "INFO", "ERROR", "WARN"),
                                Use(lambda e: getattr(logging, e))),
        "log_file_location": And(str, len),
        "granular_log_levels": [
            {
                "namespace": And(str, len),
                "level": And(str,
                             Use(str.upper),
                             lambda s: s in ("DEBUG", "INFO", "ERROR", "WARN"),
                             Use(lambda e: getattr(logging, e)))
            }
        ],
        "truncate_log_file": bool,
    }
})


class ConfigManager:
    """A class that reads, validates, and exposes the application configuration."""
    def __init__(self, cli_config):

        self.config_file = cli_config["config_file"]
        self.cli_config = cli_config
        self.config = None

        self.load()
        self.apply_cli_overrides()
        self.validate()

    def load(self):
        """Loads the configuration value into memory. The file location is derived from the command line interface
        options."""
        with open(self.config_file, "r") as f:
            self.config = json.load(f)

    def apply_cli_overrides(self):
        """Applies any command line interface overrides of configuration file values."""

        log_level = self.cli_config.get("log_level", None)
        if log_level:
            self.config["logging"]["global_log_level"] = log_level

    def validate(self):
        """Validates the configfile against the config schema."""
        self.config = CONFIG_SCHEMA.validate(self.config)


    def get_config(self):
        """Returns the validated application configuration."""
        return self.config


