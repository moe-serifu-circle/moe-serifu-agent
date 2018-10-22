from msa.event import Event




class QuitCommandEvent(Event):
    def __init__(self, data):
        super().__init__(priority=99, data=None)


