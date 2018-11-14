from typing import Dict
from schema import Schema
import datetime

class Event:
    """The base Event Class. All other events should be subclasses of this class."""

    def __init__(self, priority: int, schema: Schema):
        """Create a new event. This creates a new event and populates the event metadata but does not set the data on
        it. Data must follow the defined schema otherwise a schema.SchemaError is raised.
        Params:
        - priority (int): The priority level of this event type. Lower values indicate higher priority.
        - schema (schema.Schema): The schema for validating the data associated with this event. When `Event.init` is
        called the data object that passed in is validated against this schema."""

        self.generation_time = datetime.datetime.now()
        self.priority = priority
        self.schema = schema
        self.data = None
        self.propagate = True


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

    def init(self, data: Dict = None) -> None:
        """Sets the data property on this event. Used when creating a new event, and when deserializing an event.
        Params:
        - data (Dict): Event specific data. Must follow the defined schema for the event type."""
        self.schema.validate(data)
        self.data = data

    def get_metadata(self) -> Dict:
        """Returns the metadata of this event. Used for network serialization of an event."""
        return {
            "generation_time": self.generation_time,
            "priority": self.priority,
            "propagate": self.propagate,
        }

    def set_metadata(self, metadata: Dict) -> None:
        """Sets the metadata of this event. Used for network deserialization of an event.
        Params:
        - metadata (Dict): A dictionary containing the event metadata"""
        self.generation_time = metadata.get("generation_time", datetime.datetime.now())
        self.priority = metadata.get("priority", 100)
        self.propagate = metadata.get("propagate", True)





