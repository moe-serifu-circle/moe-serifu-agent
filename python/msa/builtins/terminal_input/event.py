from msa.event import Event


class TextInputEvent(Event):

    name = "TextInputEvent"

    def __init__(self):
        super().__init__(priority=3)


    def init(self, text):

        self.data = {"text": text}

    def load(self, data):
        self.data = data
