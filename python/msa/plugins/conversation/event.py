from msa.event import Event


class ConverseCommandEvent(Event):
    def __init__(self):
        super().__init__(priority=2)


