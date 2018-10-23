from msa.event import Event


class ConverseCommandEvent(Event):
    def __init__(self, data):
        super().__init__(2, data)

