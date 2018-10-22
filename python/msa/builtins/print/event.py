from msa.event import Event


class PrintTextEvent(Event):
    def __init__(self, text):
        super().__init__(priority=3, data={
            "text": text
        })



