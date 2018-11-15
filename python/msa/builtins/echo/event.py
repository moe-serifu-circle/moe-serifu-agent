from msa.event import Event


class EchoCommandEvent(Event):
    def __init__(self):
        super().__init__(priority=0)

    def init(self, data):
        """Parse the data provided by command's coroutine"""
        self.data = {"text": data["raw_text_input"][5:]}
