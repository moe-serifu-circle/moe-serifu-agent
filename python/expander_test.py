from msa.var import Expander
import sys, unittest


class ExpanderTest(unittest.TestCase):
    def setUp(self):
        self.ex = Expander()
        self.ex.register_var("test")
        self.ex.set_value("test", "something")

    def test_basic(self):
        self.assertEqual(self.ex.expand("$test"), "something", "expansion failed with basic input")

    def test_abuse(self):
        self.assertEqual(self.ex.expand("\\\\\$\$\\\\"), "\\$$\\", "expansion failed with escape abuse")

    def test_complex(self):
        self.assertEqual(self.ex.expand("$test, \$test"), "something, $test", "expansion failed with complex input")


if __name__ == '__main__':
    unittest.main()
