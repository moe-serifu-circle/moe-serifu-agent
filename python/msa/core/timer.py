"""
Operates system and user timers. This is used for managing repetitive tasks that are set to start in the future. Every
timer emits a particular event after a certain amount of time, and may repeat that event if scheduled to be recurring.

The Timer class is used for tracking the timing of a particular task, but this class is not intended for use on its own.
Instead, a TimerManager instance is used for maintaining each timer as well as for managing variables that apply to all
timers.
"""

from typing import Type, Mapping, Any, Sequence, Dict

import time
import logging
import random
import asyncio

import schema
from .event import Event
from . import supervisor, LogicError, ProtectionError


_log = logging.getLogger(__name__)


timer_schema = schema.Schema({
    'id': lambda x: int,
    'system': bool,
    'recurring': bool,
    'period': int,
    'last_fired': float,
    'event': {
        'class': str,
        'args': dict
    }
})

class _Timer(object):
    """
    Holds a single event construction for future execution. Keeps the time that the command is scheduled for execution
    as well as whether it is to be repeated.
    """

    def __init__(
            self, id: int, period: int, event_class: Type[Event], event_data: Mapping[str, Any], recurring: bool,
            system: bool
    ):
        """
        Create a new Timer.

        :param id: The ID of the new Timer.
        :param period: The amount of time after which the timer will fire. Used for both recurring Timers and
        non-recurring Timers.
        :param event_class: The class of the event that is to be fired. Must be a subclass of Event.
        :param event_data: The dictionary to be used as data for initializing the event.
        :param recurring: Whether the timer should be recurring. If true, after the timer initially fires, it will
        continue firing every time its period elapses.
        :param system: Whether the timer should be marked as a system (protected) timer. System timers cannot be deleted
        or modified by the end-user from the CLI, and can be used for any system tasks that need to execute on a regular
        basis. Note that enforcement of system timer protection must be handled by the user of this class.
        """
        if event_class == Event:
            raise ValueError("Cannot create a timer for Event class itself; must be a subclass")

        self.id = id
        self.is_system = system
        self.is_recurring = recurring
        self.event_class = event_class

        self._period = period
        self._last_fired = time.monotonic()
        self._event_data = dict(event_data)

    def copy(self) -> '_Timer':
        """
        Copy this Timer exactly.

        :return: A new object whose properties are the same as this Timer, including its ID.
        """
        t = _Timer(self.id, self._period, self.event_class, self._event_data, self.is_recurring, self.is_system)
        t._last_fired = self._last_fired
        return t

    def is_ready(self, now: float) -> bool:
        """
        Check whether this Timer should be fired.

        :param now: The reference point for 'now'. Given as a parameter rather than calculated so that all timers can
        use the same reference point regardless of the speed of execution of tasks, and so that missing a fire time does
        not cause schedule slip.
        :return: Whether the timer should be fired.
        """
        return self._last_fired + (self._period / 1000.0) <= now

    def fire(self, now: float) -> '_Timer':
        """
        Fire the timer and cause its event to be generated.

        :param now: The reference point for 'now'. Given as a parameter rather than calculated so that all timers can
        use the same reference point regardless of the speed of execution of tasks, and so that missing a fire time does
        not cause schedule slip.
        :return: This timer
        """

        # We already check that Event is not being instantiated in initializer, and that is only an issue as long as
        # the init() functions exist. That will be removed eventually, and the refactor will make this a non-issue.
        # noinspection PyArgumentList
        e = self.event_class()
        e.init(self._event_data)
        supervisor.fire_event(e)
        _log.debug("Fired timer %d", self.id)
        self._last_fired = now

        return self

    def serialize(self) -> Mapping[str, Any]:
        """
        Serialize the properties of this Timer to a dictionary that is safe for conversion to JSON.

        :return: The serialized properties of this Timer.
        """
        return {
            'id': self.id,
            'system': self.is_system,
            'period': self._period,
            'recurring': self.is_recurring,
            'last_fired': self._last_fired,
            'event': {
                'class': self.event_class.__name__,
                'args': self._event_data
            }
        }

    def __str__(self):
        cls_name = self.__class__.__name__
        msg = "<%s id:%s period:%dms recurring:%b system:%b %s>"
        return msg.format(cls_name, self.id, self._period, self.is_recurring, self.is_system, self.event_class.__name__)

    def __repr__(self):
        cls_name = self.__class__.__name__
        msg = "%s(%r, %r, %r, %r, %r, %r)"
        return msg.format(
            cls_name, self.id, self._period, self.event_class, self._event_data, self.is_recurring, self.is_system
        )

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        return self.id == other.id

    def __hash__(self):
        return hash(self.id)

    @staticmethod
    def deserialize(data: Mapping[str, Any]) -> '_Timer':
        """
        Create a new timer by reading properties from a data dictionary.

        :param data: The properties to use for the new _Timer. Must match the schema of a timer given in
        ``timer_schema``.
        :return: The newly-created Timer.
        """
        global timer_schema

        data = timer_schema.validate(data)
        t = _Timer(data['id'], data['period'], data['event']['class'], data['event']['args'], data['recurring'], data['system'])
        # TODO: check into compatibility of monotonic clocks
        # (python's isn't defined, so serializing 'last_fired' might not make sense; we should look into this)
        t._last_fired = data['last_fired']
        return t


class TimerManager(object):
    """
    Manages instances of Timers. Handles checking them and firing them, and removing them when no longer necessary. Uses
    the concept of 'ticks'; every regular interval of time, the timers will all be checked and fired. This interval is
    a 'tick'.

    To add a new recurring event, use the ``add_timer()`` method. To add a delayed event to fire in the future only one
    time, use the ``delay()`` method.
    """

    def __init__(self, tick_resolution: int=1):
        """
        Create a new TimerManager.

        :param tick_resolution: The length of time (in milliseconds) of a single tick.
        """
        self._last_tick_time = float("-inf")
        self._lock = asyncio.Lock()
        self._tick_resolution = tick_resolution
        self._timers: Dict[int, _Timer] = dict()
        self._available_ids = set(range(10000))  # reasonable default for max number of timers for now

    async def delay(self, delay: int, event_class: Type[Event], event_args: Mapping[str, Any]) -> int:
        """
        Delay firing of the given event until after the time has elapsed, then fire it. The event will be created with
        the given args before being fired.

        :param delay: The amount of time in milliseconds before the event should fire.
        :param event_class: The type of Event that should be fired.
        :param event_args: The arguments to the new event.
        :return: The ID of the newly-created delay-timer. This can be used to manage it prior to its firing; after the
        firing, the id will no longer be valid.
        """
        t = None
        async with self._lock:
            id = self._reserve_id()
            t = _Timer(id, delay, event_class, event_args, recurring=False, system=False)
            self._timers[id] = t
        _log.debug("Scheduled a %s event to fire in %dms (id = %d)".format(event_class, delay, id))
        return t.id

    async def add_timer(self, period: int, event_class: Type[Event], event_args: Mapping[str, Any]) -> int:
        """
        Create a timer that calls the given event every time the period elapses.

        :param period: The amount of time in milliseconds before the event should fire.
        :param event_class: The type of Event that should be fired.
        :param event_args: The arguments to the new event.
        :return: The ID of the newly-created timer. This can be used to manage it prior to it being deleted; after
        the timer is deleted, the id will no longer be valid.
        """
        async with self._lock:
            id = self._reserve_id()
            t = _Timer(id, period, event_class, event_args, recurring=True, system=False)
            self._timers[id] = t
        _log.debug("Scheduled a %s event to fire in %dms (id = %d)".format(event_class, period, id))
        return t.id

    async def add_system_timer(self, period: int, event_class: Type[Event], event_args: Mapping[str, Any]) -> int:
        """
        Create a timer that calls the given event every time the period elapses, and register it as a protected system
        timer. Note that this method grants protection to a timer, so it should not be accessible from plugins.

        :param period: The amount of time in milliseconds before the event should fire.
        :param event_class: The type of Event that should be fired.
        :param event_args: The arguments to the new event.
        :return: The ID of the newly-created timer. This can be used to manage it prior to it being deleted; after
        the timer is deleted, the id will no longer be valid.
        """
        async with self._lock:
            id = self._reserve_id()
            t = _Timer(id, period, event_class, event_args, recurring=True, system=True)
            self._timers[id] = t
        _log.debug("Scheduled a %s event to fire in %dms (id = %d)".format(event_class, period, id))
        return t.id

    async def remove_timer(self, id: int, protected: bool=True) -> None:
        """
        Remove a timer from the system.

        :param id: The ID of the timer to remove.
        :param protected: Whether protected mode is enabled, which will stop a system timer from being removed.
        """
        async with self._lock:
            try:
                t = self._timers[id]
            except KeyError:
                raise LogicError("no timer with ID: %d".format(id))
            if protected and t.is_system:
                raise ProtectionError("cannot remove system timer")
            del self._timers[id]
            self._release_id(id)

    async def get_timers(self) -> Sequence[int]:
        """
        Get the IDs of all timers currently active.

        :return: The IDs.
        """
        async with self._lock:
            timers = list(self._timers)
        return timers

    async def check_timers(self):
        """
        Check all timers and fire them if it is time to do so.

        :return:
        """
        now = time.monotonic()
        if self._last_tick_time + self._tick_resolution <= now:
            self._last_tick_time = now
            await self._fire_timers(now)

    async def _fire_timers(self, now: float):
        """
        Fires all timers.

        :param now: The current time to use for reference.
        """
        async with self._lock:
            all_timers = self._timers.copy()
            self._timers.clear()
            for tid in all_timers:
                t = all_timers[tid]
                if t.is_ready(now):
                    t.fire(now)
                    if not t.is_recurring:
                        self._release_id(tid)
                        _log.debug("Completed and removed timer %d", tid)
                    else:
                        self._timers[tid] = t


    @property
    def tick_resolution(self) -> int:
        return self._tick_resolution

    @tick_resolution.setter
    def tick_resolution(self, value: int):
        self._tick_resolution = value

    def _release_id(self, id: int):
        self._available_ids.add(id)

    def _reserve_id(self) -> int:
        ch = random.choice(self._available_ids)
        self._available_ids.remove(ch)
        return ch