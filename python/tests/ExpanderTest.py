from msa.var import Expander
import sys, unittest

class ExpanderTest(unittest.TestCase):

    def test_basic(self):
        ex = Expander()
        ex.register_var("test")
        ex.set_value("test", "something")
        self.assertEqual(ex.expand("$test"), "something")

    def test_abuse(self):
        ex = Expander()
        ex.register_var("test")
        ex.set_value("test", "something")
        self.assertEqual(ex.expand("\\\\\$\$\\\\"), "\\$$\\")

    def test_complex(self):
        ex = Expander()
        ex.register_var("test")
        ex.set_value("test", "something")
        self.assertEqual(ex.expand("$test, \$test") == "something, $test")

if __name__ == '__main__':
    unittest.main()