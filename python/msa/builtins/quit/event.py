from msa.event import Event




class QuitCommandEvent(Event):
    def __init__(self):
        super().__init__(priority=99)



