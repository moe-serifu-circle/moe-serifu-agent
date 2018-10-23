from msa.event import Event




class HelpCommandEvent(Event):
    def __init__(self, data):

        if len(data["tokens"]) > 1:
            command = data["tokens"][1]
        else:
            command = None

        super().__init__(priority=0, data={"command": command})


