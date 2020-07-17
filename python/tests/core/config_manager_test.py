import json
from msa.core.config_manager import ConfigManager
import unittest
from unittest.mock import patch, mock_open
import logging

class ConfigMangerTest(unittest.TestCase):

    def test_get_config_success(self):
        fake_config_contents = {
           "agent": {
               "name": "Masa-chan",
               "user_title": "Onee-chan"
           },
          "plugin_modules": [

          ],

          "module_config": {

          },

          "logging": {
            "global_log_level": "debug",
            "log_file_location": "msa.log",
            "truncate_log_file": False,
            "granular_log_levels": [

            ]
          }
        }
        fake_config_contents_str = json.dumps(fake_config_contents)

        cli_overrides = {
        }


        with patch("builtins.open", mock_open(read_data=fake_config_contents_str)) as mock_file:
            config_manager = ConfigManager("fake_file.json", cli_overrides)

            config = config_manager.get_config()


        assert config["agent"] == fake_config_contents["agent"]
        assert config["plugin_modules"] == fake_config_contents["plugin_modules"]
        assert config["module_config"] == fake_config_contents["module_config"]
        assert config["logging"]["global_log_level"] == "DEBUG"

        assert config["logging"]["log_file_location"] == fake_config_contents["logging"]["log_file_location"]
        assert config["logging"]["log_file_location"] == fake_config_contents["logging"]["log_file_location"]
        assert config["logging"]["truncate_log_file"] == fake_config_contents["logging"]["truncate_log_file"]
        assert config["logging"]["granular_log_levels"] == fake_config_contents["logging"]["granular_log_levels"]



    def test_override_log_level(self):
        fake_config_contents = {
           "agent": {
               "name": "Masa-chan",
               "user_title": "Onee-chan"
           },
          "plugin_modules": [

          ],

          "module_config": {

          },

          "logging": {
            "global_log_level": "debug",
            "log_file_location": "msa.log",
            "truncate_log_file": False,
            "granular_log_levels": [

            ]
          }
        }
        fake_config_contents_str = json.dumps(fake_config_contents)

        cli_overrides = {
            "log_level": "info"
        }



        with patch("builtins.open", mock_open(read_data=fake_config_contents_str)) as mock_file:
            config_manager = ConfigManager("fake_file.json", cli_overrides)

            config = config_manager.get_config()


        assert config["agent"] == fake_config_contents["agent"]
        assert config["plugin_modules"] == fake_config_contents["plugin_modules"]
        assert config["module_config"] == fake_config_contents["module_config"]
        assert config["logging"]["global_log_level"] == "INFO"

        assert config["logging"]["log_file_location"] == fake_config_contents["logging"]["log_file_location"]
        assert config["logging"]["log_file_location"] == fake_config_contents["logging"]["log_file_location"]
        assert config["logging"]["truncate_log_file"] == fake_config_contents["logging"]["truncate_log_file"]
        assert config["logging"]["granular_log_levels"] == fake_config_contents["logging"]["granular_log_levels"]













