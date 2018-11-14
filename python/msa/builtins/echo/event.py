from msa.event import Event


class EchoCommandEvent(Event):
    def __init__(self):
        super().__init__(priority=0)

    def init(self, data):
        self.data = {"text": data["raw_text_input"][5:]}
