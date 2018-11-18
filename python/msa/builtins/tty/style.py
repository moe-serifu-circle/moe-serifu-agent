from termcolor import colored

class TextColors:
    RED = "red"
    BLUE = "blue"
    GREY = "grey"
    GREEN = "green"
    YELLOW = "yellow"
    MAGENTA = "magenta"
    CYAN = "cyan"
    WHITE = "white"


class TextHighlights:
    RED = "on_red"
    BLUE = "on_blue"
    GREY = "on_grey"
    GREEN = "on_green"
    YELLOW = "on_yellow"
    MAGENTA = "on_magenta"
    CYAN = "on_cyan"
    WHITE = "on_white"


class TextAttributes:
    BOLD = "bold"
    UNDERLINE = "underline"


def _style_text(text_chunk):
    output = ""
    for chunk in text_chunk:
        output += colored(chunk["text"], chunk["color"], attrs=chunk.get("attrib"))
    return output


def heading(text):
    return {
        "text": text + ":\n",
        "color": TextColors.WHITE,
        "attrib": [TextAttributes.BOLD]
    }


def definition(title, description):
    out = [{
        "text": title + ": ",
        "color": TextColors.WHITE,
        "attrib": [TextAttributes.BOLD]
    }, {
        "text": description + "\n",
        "color": TextColors.YELLOW
    }]
    return out


def styled_text(text, color, attrs=None):
    return {
        "text": text,
        "color": color,
        "attrs": attrs
    }


