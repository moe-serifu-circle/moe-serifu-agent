import json
import logging
from types import MappingProxyType
from schema import Schema, And, Or, Use

CONFIG_SCHEMA = Schema(
    {
        "agent": {"name": And(str, len), "user_title": And(str, len)},
        "plugin_modules": [And(str, len)],
        "module_config": And(dict),
            # namespace of module -> dict of config values

        "logging": {
            "global_log_level": Or(
                None,
                And(
                    str,
                    Use(str.upper),
                    lambda s: s in ("DEBUG", "INFO", "ERROR", "WARN"),
                ),
            ),
            "log_file_location": And(str, len),
            "granular_log_levels": [
                {
                    "namespace": And(str, len),
                    "level": And(
                        str,
                        Use(str.upper),
                        lambda s: s in ("DEBUG", "INFO", "ERROR", "WARN"),
                        Use(lambda e: getattr(logging, e)),
                    ),
                }
            ],
            "truncate_log_file": bool,
        },
    }
)


class ConfigManager:
    """A class that reads, validates, and exposes the application configuration."""

    def __init__(self, config_file, cli_overrides):

        self.config_file = config_file
        self.cli_overrides = cli_overrides
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
        for key, value in self.cli_overrides.items():
            if key == "log_level" and value is not None:
                self.config["logging"]["global_log_level"] = value

    def validate(self):
        """Validates the configfile against the config schema."""
        self.config = CONFIG_SCHEMA.validate(self.config)

    def get_config(self):
        """Returns the validated application configuration."""
        return self.config
