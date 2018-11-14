import datetime

class Event:

    def __init__(self, priority, schema):

        self.generation_time = datetime.datetime.now()
        self.priority = priority
        self.schema = schema
        self.data = None
        self.propogate = True


    def __eq__(self, other):
        return (
            other is not None
            and self.__class__ == other.__class__
            and self.priority == other.priority
        )


    def __lt__(self, other):
        return (
            other is not None
            and self.__class__ == other.__class__
            and self.priorty < other.priority
        )

    def __le__(self, other):
        return (
            other is not None
            and self.__class__ == other.__class__
            and self.priority <= other.priority
        )

    def __ne__(self, other):
        return (
            other is not None
            and self._class__ == other.__class__
            and self.priority != other.priority
        )

    def __gt__(self, other):
        return (
            other is not None
            and self._class__ == other.__class__
            and self.priority > other.priority
        )

    def __ge__(self, other):
        return (
            other is not None
            and self.__class__ == other.__class__
            and self.priority >= other.priority
        )

    def init(self, data=None):
        self.schema.validate(data)
        self.data = data



