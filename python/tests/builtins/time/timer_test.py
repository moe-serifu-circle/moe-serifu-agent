import unittest
import logging
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock, AsyncMockWithArgs

from msa.core import LogicError, ProtectionError

from msa.builtins.time.events import TimerCommandEvent
from msa.core.event import Event
from msa.builtins.tty.events import TextInputEvent
from msa.builtins.time.timer import _Timer

import time


class TimerTests(unittest.TestCase):

    def setUp(self):
        pass

    def test_timer__instantiation(self):
        # variable sets
        cases = [
            {'id': -2,      'period': 2500, 'event': TextInputEvent,    'recurring': True,  'system': True, 'data': {}},
            {'id': 0,       'period': 1,    'event': TextInputEvent,    'recurring': False, 'system': True, 'data': {'message': 'test'}},
            {'id': 1824024, 'period': 8,    'event': TimerCommandEvent, 'recurring': True,  'system': False, 'data': {'raw_text': 'text', 'tokens': ['text']}}
        ]

        for case_data in cases:
            tid = case_data['id']
            period = case_data['period']
            event = case_data['event']
            recurring = case_data['recurring']
            system = case_data['system']
            event_data = case_data['data']

            # act
            t = _Timer(tid, period, event, event_data, recurring, system)

            # assert
            self._assert_property_equals('ID', t.id, tid)
            self._assert_property_equals('Period', t._period, period)
            self._assert_property_equals('Event Class', t.event_class, event)
            self._assert_property_equals('Event Data', t._event_data, event_data)
            self._assert_property_equals('Recurring', t.is_recurring, recurring)
            self._assert_property_equals('System', t.is_system, system)

    def test_timer__instantiation__do_not_allow_raw_event(self):
        self.assertRaises(ValueError, lambda: _Timer(0, 1, Event, {}, recurring=False, system=False))

    def test_timer__instantiation__do_not_allow_period_0(self):
        self.assertRaises(ValueError, lambda: _Timer(0, 0, TextInputEvent, {}, recurring=False, system=False))

    def test_timer__copy(self):
        # variable sets
        cases = [
            {'timer': _Timer(12, 16, TextInputEvent, {}, True, False)},
            {'timer': _Timer(0, 2000, TextInputEvent, {'message': 'msg'}, True, False)},
            {'timer': _Timer(-12, 16, TimerCommandEvent, {}, False, True)},
        ]

        for case_data in cases:
            t1 = case_data['timer']

            t2 = t1.copy()

            self._assert_property_equals('ID', t2.id, t1.id, case_data)
            self._assert_property_equals('Period', t2._period, t1._period, case_data)
            self._assert_property_equals('Event Class', t2.event_class, t1.event_class, case_data)
            self._assert_property_equals('Event Data', t2._event_data, t1._event_data, case_data)
            self._assert_property_equals('Recurring', t2.is_recurring, t1.is_recurring, case_data)
            self._assert_property_equals('System', t2.is_system, t1.is_system, case_data)
            self.assertEqual(t2, t1)

    def test_timer__is_ready(self):
        # static data
        now = time.monotonic()

        # variable sets
        cases = [
            {'period': 5,       'last_fired_diff_from_now': -1,     'expected_result': False},
            {'period': 5,       'last_fired_diff_from_now': -4.5,   'expected_result': False},
            {'period': 5,       'last_fired_diff_from_now': -4.75,  'expected_result': False},
            {'period': 5,       'last_fired_diff_from_now': -5,     'expected_result': True},
            {'period': 5,       'last_fired_diff_from_now': -5.25,  'expected_result': True},
            {'period': 5,       'last_fired_diff_from_now': -5.5,   'expected_result': True},
            {'period': 5,       'last_fired_diff_from_now': -10,    'expected_result': True},
            {'period': 10000,   'last_fired_diff_from_now': -9999,  'expected_result': False},
            {'period': 10000,   'last_fired_diff_from_now': -10000, 'expected_result': True},
            {'period': 10000,   'last_fired_diff_from_now': -10001, 'expected_result': True}
        ]

        for case_data in cases:
            period = case_data['period']
            last_fired_diff_from_now = case_data['last_fired_diff_from_now']
            expected_result = case_data['expected_result']

            t = _Timer(0, period, TextInputEvent, {}, False, False)
            t._last_fired = now - last_fired_diff_from_now

            self.assertEqual(t.is_ready(now), expected_result, '')

            self._assert_property_equals('ID', t2.id, t1.id)
            self._assert_property_equals('Period', t2._period, t1._period)
            self._assert_property_equals('Event Class', t2.event_class, t1.event_class)
            self._assert_property_equals('Event Data', t2._event_data, t1._event_data)
            self._assert_property_equals('Recurring', t2.is_recurring, t1.is_recurring)
            self._assert_property_equals('System', t2.is_system, t1.is_system)
            self.assertEqual(t2, t1)

    def _assert_property_equals(self, prop_name, prop_value, expected_value, case_data=None):
        err_msg = '{:s} failed: expected {!r:s} but actual property is {!r:s}'
        err_msg = err_msg.format(prop_name, expected_value, prop_value)
        if case_data is not None:
            err_msg = _case_message(err_msg, case_data)
        self.assertEqual(prop_value, expected_value, err_msg)


def _case_message(msg, case_data):
    msg = '{:s} (failed for case {!s:s})'.format(msg, case_data)
    return msg


def test_case(**kwargs):
    def inner(func):
        def wrap(self, *args):
            if not hasattr(self, '__test_cases'):
                self.__current_test_cases = {}
            if func.__name__ not in self.__current_test_cases:
                self.__current_test_cases[func.__name__]
            func(self, *args, **kwargs)
        wrap.test_cases = []

        return wrap
    return inner


if __name__ == "__main__":
    unittest.main()
