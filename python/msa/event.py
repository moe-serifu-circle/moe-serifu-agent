import datetime

class Event:

    def __init__(self, event_type, priority, data=None):

        self.type = event_type
        self.generation_time = datetime.datetime.now()
        self.priority = priority
        self.data = data
        self.propogate = True


    def __eq__(self, other):
        return self.priority == other.priority

    def __lt__(self, other):
        return self.priority < other.priority

    def __le__(self, other):
        return self.priority <= other.priority

    def __ne__(self, other):
        return self.priorty != other.priority

    def __gt__(self, other):
        return self.priority > other.priority

    def __ge__(self, other):
        return self.priority >= other.priority
