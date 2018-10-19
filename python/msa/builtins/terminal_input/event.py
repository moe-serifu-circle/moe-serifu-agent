from msa.event import Event


class TextInputEvent(Event):
    def __init__(self, text):
        super().__init__("TextInputEvent", priority=3, data={
            "text": text
        })



