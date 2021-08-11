from __future__ import annotations
from typing import Dict, Union, Type
from schema import Schema
import datetime


class Event(object):
    """The base Event Class. All other events should be subclasses of this class."""

    def __init__(self, priority: int, schema: Schema):
        """Create a new event. This creates a new event and populates the event metadata but does not set the data on
        it. Data must follow the defined schema otherwise a schema.SchemaError is raised.

        Parameters
        ----------
        priority : int
            The priority level of this event type. Lower values indicate higher priority.
        schema : schema.Schema
            The schema for validating the data associated with this event. When `Event.init` is
            called the data object that passed in is validated against this schema."""

        self.generation_time = datetime.datetime.now()
        self.priority = priority
        self.schema = schema
        self.data = None
        self.propagate = True
        self._network_propagate = False
        self.propagate_target = "all"
        self.propagate_source = None

    def __eq__(self, other):
        return (
            other is not None
            and self.__class__ == other.__class__
            and self.data == other.data
        )

    def __ne__(self, other):
        return not self == other

    def __lt__(self, other):
        """" Note: the inequality is backwards on purpose because asyncio.PriorityQueue uses low values as higher priority"""
        return (
            other is not None
            and isinstance(other, Event)
            and self.priority > other.priority
        )

    def __le__(self, other):
        """" Note: the inequality is backwards on purpose because asyncio.PriorityQueue uses low values as higher priority"""
        return (
            other is not None
            and isinstance(other, Event)
            and self.priority >= other.priority
        )

    def __gt__(self, other):
        """" Note: the inequality is backwards on purpose because asyncio.PriorityQueue uses low values as higher priority"""
        return (
            other is not None
            and isinstance(other, Event)
            and self.priority < other.priority
        )

    def __ge__(self, other):
        """" Note: the inequality is backwards on purpose because asyncio.PriorityQueue uses low values as higher priority"""
        return (
            other is not None
            and isinstance(other, Event)
            and self.priority <= other.priority
        )

    def init(self, data: Union[Dict, None] = None) -> Type[Event]:
        """
        Sets the data property on this event. Used when creating a new event, and when deserializing an event.

        :param Dict data: Event specific data. Must follow the defined schema for the event type.
        :return: the initialized event instance
        """
        """

        Parameters
        ----------
        data : Dict
            """
        self.data = self.schema.validate(data)

        return self

    def get_metadata(self) -> Dict:
        """Returns the metadata of this event. Used for network serialization of an event."""
        return {
            "event_type": self.__class__.__name__,
            "generation_time": self.generation_time.strftime("%Y-%m-%d %H:%M:%S.%f"),
            "priority": self.priority,
            "propagate": self.propagate,
            "_network_propagate": self._network_propagate,
            "propagate_source": self.propagate_source,
            "propagate_target": self.propagate_target,
            "event_data": self.data,
        }

    def set_metadata(self, metadata: Dict) -> None:
        """Sets the metadata of this event. Used for network deserialization of an event.

        Parameters
        ----------
        metadata : Dict
            A dictionary containing the event metadata"""
        self.generation_time = metadata.get("generation_time", datetime.datetime.now())
        self.priority = metadata.get("priority", 100)
        self.propagate = metadata.get("propagate", True)
        self._network_propagate = metadata.get("_network_propagate", False)
        self.data = metadata.get("event_data", None)
        self.propagate_source = metadata.get("propagate_source")
        self.propagate_target = metadata.get("propagate_target")
        self.schema.validate(self.data)

    def __str__(self):
        return f"<{self.__class__.__module__}.{self.__class__.__name__} at {hex(id(self))} created at {self.generation_time}"

    def network_propagate(self):
        """
        Indicate that the event should be propagated across the network.

        :return: the Event instance
        """
        self._network_propagate = True
        return self

    def source(self, source):
        """
        Indicate the source of the event

        :param source: A client id to specify the source of the event
        :return: the Event instance
        """
        self.propagate_source = source
        return self

    def target(self, target):
        """
        Indicate the target for the event. By default the target is set to "all". A target of "all" indicates that
        the event should be propagated to all clients.

        :param target: A client id or "all"
        :return: the Event instance
        """
        self.propagate_target = target
        return self

    @staticmethod
    def deserialize(event_data):
        event_type = event_data.get("event_type", None)
        subclasses = Event.__subclasses__()

        if not event_type:
            raise Exception(
                "Attempted to deserialize an event that does not have a defined event type."
            )

        for cls in subclasses:
            if event_type == cls.__name__:
                new_event = cls()
                new_event.set_metadata(event_data)
                return new_event

        raise Exception(
            f"Attempted to deserialize an event of type {event_type} but an event class of that type could not be found."
        )
