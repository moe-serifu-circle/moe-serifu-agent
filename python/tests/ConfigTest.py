from msa.config import *
import unittest
# only used * for simple namespace usage within this file only
# don't use in other files if possible as a general rule


class ConfigTest(unittest.TestCase):

    def test_config(self):
        cfg = load("tests/msaExample.cfg")
        print("loaded default config file")
        cfg.sections["agent"].push("user_title", "Onii-chan")  # testing multiple values
        save(cfg, "tests/test.cfg")  # testing save function
        print("saved new config file, check file to confirm correct output")

        new_cfg = load("tests/test.cfg")  # testing loading with multiple values
        self.assertTrue(len(new_cfg.sections["agent"].get_all("user_title")) == 2, "test failed with multiple value assignments")


if __name__ == '__main__':
    unittest.main()