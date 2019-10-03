import time
import traceback
import os
import asyncio
import webbrowser

from prompt_toolkit import PromptSession
from prompt_toolkit.eventloop.defaults import use_asyncio_event_loop
from prompt_toolkit.lexers import PygmentsLexer
from prompt_toolkit.history import FileHistory
from pygments.lexers.python import Python3Lexer, Python3TracebackLexer
from pygments.formatters import TerminalFormatter
from pygments import highlight

from msa.api import get_api, run_async
from msa.api.context import ApiContext
from msa.core.config_manager import ConfigManager

use_asyncio_event_loop()



class Interpreter:
    def __init__(self):
        history_file = FileHistory('.msa_cli_history')
        self.prompt_session = PromptSession(
                lexer=PygmentsLexer(Python3Lexer),
                history=history_file)

        self.config_manager = ConfigManager({"config_file": "msa_config.json"})
        self.config = self.config_manager.get_config()

        self.api = get_api(ApiContext.rest, self.config["plugin_modules"], host="localhost", port=8080)

        self.quit = False
        self.exit_code = 0

        self.locals = {}
        self.globals = {
            "msa_api":  self.api
        }
        self.func_locals = {}
        self.buffer = ""
        self.indent_level = 0
        self.indent_size = 4

        self.recording = False
        self.record_buffer_name = ""
        self.record_buffer = ""



    async def startup_check(self):
        await self.api.check_connection()
        await self.api.check_version(quiet=True)


    def execute_script(self, script):
        with open(script, "r") as f:
            text = f.read()
            print("Running script:")
            print("| " + "\n| ".join(text.split("\n")))
            print("Script output:")
            self.execute_block(text)

    def start(self):
        asyncio.get_event_loop().run_until_complete(self._start())

    async def _start(self):
        # startup checks
        try:
            await self.api.client.connect()
            await self.startup_check()

            while True:
                try:
                    prompt_text, prompt_default = self.generate_prompt_text()
                    text = await self.prompt_session.prompt(prompt_text, default=prompt_default, async_=True)

                except KeyboardInterrupt:
                    continue
                except EOFError:
                    break
                except Exception as e:
                    print(e)
                    time.sleep(5)
                else:
                    await self.parse_statement(text)

                if self.quit:
                    break

            print("Goodbye")
            quit(self.exit_code)
        finally:
            await self.api.client.disconnect()

    def generate_prompt_text(self):
        if self.indent_level == 0:
            prompt_text = '>>> '
            prompt_default = ""
        else:
            prompt_text = '...'
            prompt_default = ' '*self.indent_level*self.indent_size

        return prompt_text, prompt_default


    async def parse_statement(self, text):
        if self.indent_level == 0:
            
            skip_loop = self.parse_command(text)
            if skip_loop: return 

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
        
            await self.execute_block(text)

    async def execute_block(self, text):
        try:
            await self.aexec(text.strip())
        except SystemExit as e:
            self.quit = True
            self.exit_code = e.code
            return
        except:
            self.print_traceback(traceback.format_exc())

    async def aexec(self, code):
        # Make an async function with the code and `exec` it

        effective_globals = {**self.func_locals, **self.globals}
        exec(
            f'async def __ex(): ' +
            ''.join(f'\n {l}' for l in code.split('\n')) + "\n return locals()",
        effective_globals, self.locals)

        # Get `__ex` from local variables, call it and return the result
        self.func_locals = await self.locals['__ex']()

        for key in list(self.func_locals.keys()):
            if key in self.globals:
                del self.func_locals[key]
                raise Exception(f"Statement attempted to override global variable \"{key}\". This is not allowed.")


    def print_traceback(self, *args, **kwargs):
        stringify = ' '.join(str(e) for e in args)
        print(highlight(stringify, Python3TracebackLexer(), TerminalFormatter()))

    def parse_command(self, text):
        

        clean_text = text.strip()
        if len(clean_text) == 0: # obviously there is no command to parse 
            return

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
                    if editor == None or editor == "":
                        print("Opening {}".format(self.record_buffer_name))
                        webbrowser.open("file://" + os.path.abspath(self.record_buffer_name))
                    else:
                        print("Opening {} via {}".format(self.record_buffer_name, editor))
                        os.system('%s %s' % (editor, self.record_buffer_name))

                    return True
                else:
                    if len(tokens) < 2:
                        print("Record command requires a file name to record to. e.g. #record a.py")
                        return True

                    self.record_buffer_name = tokens[1]
                    self.recording = True
                    return True
            elif tokens[0] == "clear":
                print(chr(27) + "[2J")

            elif tokens[0] == "help":
                print('\n'.join(("MSA Interpreter Help:",
                        " Availiable Commands:",
                        "  # help: Show this help text",
                        "  # record <file name>: Begin recording commands to a script.",
                        "  # record stop: stop recording commands, save the script, and open to review.")))

        return False





