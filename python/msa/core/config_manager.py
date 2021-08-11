import json
import logging
from types import MappingProxyType
from schema import Schema, And, Or, Use
from pathlib import Path

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
    """A class that reads, validates, and exposes the application configuration.


    :param cli_overrides A dictionary of overrides for various config values.
        Supported config overrides:
        - *config_file*: The path to the msa config file. If not provided or the provided value is not a valid path to
        a file, ConfigManager will search $HOME/.msa_config.json and ./msa_config.json in that order.
        - *log_level* Override the root log level of MSA. Valid values are "debug", "info", "warn", "error"
    """

    def __init__(self, cli_overrides):

        self.config_file = cli_overrides.get("config_file", None)
        self.cli_overrides = cli_overrides
        self.config = None

        self.logger = logging.getLogger("msa.core.config_manager.ConfigManager")

        self.load()
        self.apply_cli_overrides()
        self.validate()

    def load(self):
        """Loads the configuration value into memory. The file location is derived from the command line interface
        options."""

        paths_to_check = [Path("./msa_config.json"), Path.home() / ".msa_config.json"]
        if self.config_file is not None:
            paths_to_check.insert(0, Path(self.config_file))

        success = False
        for path in paths_to_check:
            self.logger.info(f"Searching path for msa config file: '{path}'")
            if not path.exists():
                self.logger.warning(
                    f"Provided config file path: {path} does not exist."
                )
                continue

            if not path.is_file():
                self.logger.warning(f"Provided config file path: {path} is not a file.")
                continue

            with path.open("r") as f:
                self.logger.info(f"Successfully loaded config file: {path}")
                self.config = json.load(f)
                success = True
                break

        if not success:
            msg = "Failed for find a valid msa config file."
            self.logger.error(msg)
            raise MissingConfigFileException(msg)

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


class MissingConfigFileException(Exception):
    pass
