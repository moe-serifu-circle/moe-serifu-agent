import unittest

from msa.builtins.tty.style import _style_text
from msa.builtins.tty.style import *


class StyleTests(unittest.TestCase):

    def test_heading(self):

        raw_text = "test message"

        actual = heading(raw_text)

        expected = {
            "text": raw_text + ":\n",
            "color": TextColors.WHITE,
            "attrib": [TextAttributes.BOLD]
        }

        self.assertEqual(expected, actual)

    def test_definition(self):
        title = "test message"
        description = "description"

        actual = definition(title, description)

        expected = [{
            "text": title + ": ",
            "color": TextColors.WHITE,
            "attrib": [TextAttributes.BOLD]
        }, {
            "text": description + "\n",
            "color": TextColors.YELLOW
        }]

        self.assertEqual(expected, actual)


    def test_styled_text(self):
        text = "test message"
        color = TextColors.CYAN
        attrib = [TextAttributes.BOLD, TextAttributes.UNDERLINE]

        actual = styled_text(text, color, attrib)

        expected = {
            "text": text,
            "color": color,
            "attrs": attrib
        }

        self.assertEqual(expected, actual)


    def test_style_text(self):
        text = "test message"
        color = TextColors.CYAN
        attrib = [TextAttributes.BOLD, TextAttributes.UNDERLINE]


        input = [{
            "text": text,
            "color": color,
            "attrs": attrib
        }]

        result = _style_text(input)

        self.assertTrue(text in result)







if __name__ == "__main__":
    unittest.main()