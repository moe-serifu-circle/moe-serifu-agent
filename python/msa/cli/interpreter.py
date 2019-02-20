import traceback
import os
import webbrowser

from prompt_toolkit import PromptSession
from prompt_toolkit.lexers import PygmentsLexer
from pygments.lexers.python import Python3Lexer, Python3TracebackLexer
from pygments.formatters import TerminalFormatter
from pygments import highlight


class Interpreter:
    def __init__(self):
        self.prompt_session = PromptSession(
                lexer=PygmentsLexer(Python3Lexer))

        self.locals = {}
        self.globals = {
        }
        self.buffer = ""
        self.indent_level = 0
        self.indent_size = 4

        self.recording = False
        self.record_buffer_name = ""
        self.record_buffer = ""

    def start(self):

        while True:
            try:
                if self.indent_level == 0:
                    prompt_text = '>>> '
                    prompt_default = ""
                else:
                    prompt_text = '...'
                    prompt_default = ' '*self.indent_level*self.indent_size

                text = self.prompt_session.prompt(prompt_text, default=prompt_default)

            except KeyboardInterrupt:
                continue
            except EOFError:
                break
            else:

                if self.indent_level == 0:
                    skip_loop = self.parse_command(text)
                    if skip_loop: continue

                if self.recording:
                    self.record_buffer += text + "\n"

                # update current indent level
                if self.indent_level > 0:
                    if len(text.strip()) == 0:
                        self.indent_level = 0
                    else:
                        current_indent = (len(text) - len(text.lstrip()))//self.indent_size
                        self.indent_level = current_indent

                # if we see a colon, begin buffer
                if len(text) > 0 and text[-1] == ":":
                    self.indent_level += 1

                if self.indent_level > 0:
                    self.buffer += text + "\n"

                if self.indent_level == 0:
                    if len(self.buffer) > 0:
                        text = self.buffer + "\n" + text
                        self.buffer = ""


                    try:
                        exec(text.strip() ,self.globals, self.locals)
                    except SystemExit:
                        break
                    except:
                        self.print_traceback(traceback.format_exc())



        print("Goodbye")

    def print_traceback(self, *args, **kwargs):
        stringify = ' '.join(str(e) for e in args)
        print(highlight(stringify, Python3TracebackLexer(), TerminalFormatter()))

    def parse_command(self, text):
        clean_text = text.strip()
        if clean_text[0] == "#":
            tokens = clean_text[1::].split()


            if tokens[0] == "record":
                if len(tokens) < 2:
                    print("Record command requires either 'stop' or a file name to record to.")
                    return True

                if tokens[1] == "stop":
                    self.recording = False

                    with open(self.record_buffer_name,"w") as f:
                        f.write(self.record_buffer)
                    self.record_buffer = ""

                    editor = os.getenv("EDITOR")
                    if editor:
                        os.system('%s %s' % (editor, self.record_buffer_name))
                    else:
                        webbrowser.open(self.record_buffer_name)

                    return True
                else:
                    if len(tokens) < 2:
                        print("Record command requires a file name to record to. e.g. #record a.py")
                        return True

                    self.record_buffer_name = tokens[1]
                    self.recording = True
                    return True

        return False





