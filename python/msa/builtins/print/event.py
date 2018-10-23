from msa.event import Event


class PrintTextEvent(Event):
    def __init__(self):
        super().__init__(priority=3)


    def init(self, text):
        self.data = {"text": text}

