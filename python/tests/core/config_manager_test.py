import json
from msa.core.config_manager import ConfigManager
import unittest
from unittest.mock import patch, mock_open, MagicMock
import logging


class ConfigMangerTest(unittest.TestCase):
    @patch("msa.core.config_manager.Path")
    def test_get_config_success(self, Path_mock: MagicMock):
        fake_config_contents = {
            "agent": {"name": "Masa-chan", "user_title": "Onee-chan"},
            "plugin_modules": [],
            "module_config": {},
            "logging": {
                "global_log_level": "debug",
                "log_file_location": "msa.log",
                "truncate_log_file": False,
                "granular_log_levels": [],
            },
        }
        fake_config_contents_str = json.dumps(fake_config_contents)

        cli_overrides = {}

        local_dir_path_mock = MagicMock()
        custom_path_mock = MagicMock()

        local_dir_path_mock.exists = MagicMock(return_value=True)
        local_dir_path_mock.open = mock_open(read_data=fake_config_contents_str)

        path_mocks = {"./msa_config.json": local_dir_path_mock}

        Path_mock.side_effect = lambda arg: path_mocks[arg]

        home_path_mock = MagicMock()
        home_path_extended_mock = MagicMock()
        home_path_extended_mock.exists = MagicMock(return_value=False)
        home_path_mock.join = MagicMock(return_value=home_path_extended_mock)
        Path_mock.home.return_value = home_path_mock

        config_manager = ConfigManager(cli_overrides)

        config = config_manager.get_config()

        self.assertEqual(config["agent"], fake_config_contents["agent"])
        self.assertEqual(
            config["plugin_modules"], fake_config_contents["plugin_modules"]
        )
        self.assertEqual(config["module_config"], fake_config_contents["module_config"])
        self.assertEqual(config["logging"]["global_log_level"], "DEBUG")

        self.assertEqual(
            config["logging"]["log_file_location"],
            fake_config_contents["logging"]["log_file_location"],
        )
        self.assertEqual(
            config["logging"]["log_file_location"],
            fake_config_contents["logging"]["log_file_location"],
        )
        self.assertEqual(
            config["logging"]["truncate_log_file"],
            fake_config_contents["logging"]["truncate_log_file"],
        )
        self.assertEqual(
            config["logging"]["granular_log_levels"],
            fake_config_contents["logging"]["granular_log_levels"],
        )

    @patch("msa.core.config_manager.Path")
    def test_override_log_level(self, Path_mock: MagicMock):
        fake_config_contents = {
            "agent": {"name": "Masa-chan", "user_title": "Onee-chan"},
            "plugin_modules": [],
            "module_config": {},
            "logging": {
                "global_log_level": "debug",
                "log_file_location": "msa.log",
                "truncate_log_file": False,
                "granular_log_levels": [],
            },
        }
        fake_config_contents_str = json.dumps(fake_config_contents)

        cli_overrides = {"log_level": "info", "config_file": "fake_file.json"}

        local_dir_path_mock = MagicMock()
        custom_path_mock = MagicMock()

        local_dir_path_mock.exists = MagicMock(return_value=False)
        custom_path_mock.exists = MagicMock(return_value=True)
        custom_path_mock.open = mock_open(read_data=fake_config_contents_str)

        path_mocks = {
            "./msa_config.json": local_dir_path_mock,
            "fake_file.json": custom_path_mock,
        }

        Path_mock.side_effect = lambda arg: path_mocks[arg]

        home_path_mock = MagicMock()
        home_path_extended_mock = MagicMock()
        home_path_extended_mock.exists = MagicMock(return_value=False)
        home_path_mock.join = MagicMock(return_value=home_path_extended_mock)
        Path_mock.home.return_value = home_path_mock

        config_manager = ConfigManager(cli_overrides)

        config = config_manager.get_config()

        self.assertEqual(config["agent"], fake_config_contents["agent"])
        self.assertEqual(
            config["plugin_modules"], fake_config_contents["plugin_modules"]
        )
        self.assertEqual(config["module_config"], fake_config_contents["module_config"])
        self.assertEqual(config["logging"]["global_log_level"], "INFO")

        self.assertEqual(
            config["logging"]["log_file_location"],
            fake_config_contents["logging"]["log_file_location"],
        )
        self.assertEqual(
            config["logging"]["log_file_location"],
            fake_config_contents["logging"]["log_file_location"],
        )
        self.assertEqual(
            config["logging"]["truncate_log_file"],
            fake_config_contents["logging"]["truncate_log_file"],
        )
        self.assertEqual(
            config["logging"]["granular_log_levels"],
            fake_config_contents["logging"]["granular_log_levels"],
        )
