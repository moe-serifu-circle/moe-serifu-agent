
from msa.core.event_handler import EventHandler

from msa.builtins.command_registry.events import RegisterCommandEvent, HelpCommandEvent
from msa.builtins.tty.events import TextInputEvent
from msa.core import supervisor


class CommandRegistryHandler(EventHandler):

    def __init__(self, loop, event_queue):
        super().__init__(loop, event_queue)

        self.registered_commands = {}

    async def handle(self):

        _, event = await self.event_queue.get()

        if not event.propagate:
            return

        if isinstance(event, RegisterCommandEvent):
            self.register_command(event.data)

        elif isinstance(event, TextInputEvent):
            self.parse_text_input(event)

    def register_command(self, data):

        # verify that no other commands utilize the same invoke keyword
        keyword = data["invoke"].lower()
        if keyword in self.registered_commands:
            print("Command with invoke keyword '{}' already registered".format(keyword))
            return

        self.registered_commands[keyword] = data


    def parse_text_input(self, event):
        raw_text = event.data.get("message")


        if raw_text is None or not len(raw_text):
            return

        tokens = raw_text.split()
        invoke_keyword = tokens[0]

        # search registered commands for one that corresponds to invoke_keyword
        for command_type, command_data in self.registered_commands.items():
            if invoke_keyword == command_type:
                new_event = command_data["event_constructor"]()
                new_event.init(data={
                    "raw_text": raw_text,
                    "tokens": tokens[1:len(tokens)]
                })

                supervisor.fire_event(new_event)
                return




class HelpCommandHandler(EventHandler):

    def __init__(self, loop, event_queue):
        super().__init__(loop, event_queue)

        self.registered_commands = {}

    async def init(self):
        event = RegisterCommandEvent()
        event.init({
            "event_constructor": HelpCommandEvent,
            "invoke": "help",
            "describe": "Prints available commands and information about command usage.",
            "usage": "'help'  or 'help [command name]'",
            "options": "No options available."
        })


        supervisor.fire_event(event)


    async def handle(self):

        _, event = await self.event_queue.get()


        if not event.propagate:
            return

        if isinstance(event, RegisterCommandEvent):
            self.register_command(event)

        elif isinstance(event, HelpCommandEvent):
            self.display_help(event)

    def register_command(self, event):

        data = event.data

        # verify that no other commands utilize the same invoke keyword
        keyword = data["invoke"].lower()
        if keyword in self.registered_commands:
            print("Command with invoke keyword '{}' already registered".format(keyword))
            return

        self.registered_commands[keyword] = data


    def display_help(self, event):
        tokens = event.data.get("tokens")

        if tokens is None or not len(tokens):
            # print availiable commands
            out = "Availiable Commands: \n"

            for command_name, command_data in self.registered_commands.items():
                out += "{}: {}\n".format(command_name, command_data["describe"])
            out += "\n"

            print(out) # TODO refactor to use TTY output event

        else:
            command = tokens[0]

            for command_name, command_data in self.registered_commands.items():
                if command == command_name:
                    out = "Help text for command '{}':\nUsage: {}\nOptions: {}\nDescription: {}\n".format(
                        command_name,
                        command_data["usage"],
                        command_data["options"],
                        command_data["describe"],
                    )

                    print(out) # TODO refactor to use TTY output event
                    return
