import unittest

import schema
from schema import Schema, And

from msa.core.event import Event

class EventTest(unittest.TestCase):

    def test_init_valid_data(self):

        new_event = DummyEvent()

        data = {
            "prop_1": 5,
            "prop_2": "ASDF"
        }

        new_event.init(data)

    def test_init_bad_int(self):

        new_event = DummyEvent()

        data = {
            "prop_1": "5",
            "prop_2": "ASDF"
        }

        self.assertRaises(
            schema.SchemaError,
            new_event.init,
            data
        )

    def test_init_empty_str(self):

        new_event = DummyEvent()

        data = {
            "prop_1": 5,
            "prop_2": ""
        }

        self.assertRaises(
            schema.SchemaError,
            new_event.init,
            data
        )

    def test_init_bad_str(self):

        new_event = DummyEvent()
        data = {
            "prop_1": 5,
            "prop_2": 123
        }

        self.assertRaises(
            schema.SchemaError,
            new_event.init,
            data
        )

    def test_equality(self):

        a = FakeA().init({"key": 1})
        a_2 = FakeA().init({"key": 2})
        a_3 = FakeA().init({"key": 1})

        b = FakeB()

        assert a != a_2
        assert a == a_3
        assert a != b
        assert a == a

    def test_less_than(self):

        high = HighPriority()
        medium = MediumPriority()
        low = LowPriority()

        assert not high < medium
        assert medium < high

        assert not high < low
        assert low < high

        assert not medium < low
        assert low < medium

    def test_less_than_equals(self):

        high = HighPriority()
        high2 = HighPriority()
        medium = MediumPriority()
        medium2 = MediumPriority()
        low = LowPriority()
        low2 = LowPriority()

        assert high <= high2
        assert not high <= medium
        assert medium <= high

        assert medium <= medium2
        assert not high <= low
        assert low <= high

        assert low <= low2
        assert not medium <= low
        assert low <= medium
        
    def test_greater_than(self):

        high = HighPriority()
        medium = MediumPriority()
        low = LowPriority()

        assert high > medium
        assert not medium > high

        assert high > low
        assert not low > high

        assert medium > low
        assert not low > medium

    def test_greater_than_equals(self):

        high = HighPriority()
        high2 = HighPriority()
        medium = MediumPriority()
        medium2 = MediumPriority()
        low = LowPriority()
        low2 = LowPriority()

        assert high >= high2
        assert high >= medium
        assert not medium >= high

        assert medium >= medium2
        assert high >= low
        assert not low >= high

        assert low >= low2
        assert medium >= low
        assert not low >= medium


    def test_get_metadata_and_deserialize(self):
        dummy_event = DummyEvent().init({"prop_1": 1, "prop_2": "hello"})

        raw_dict = dummy_event.get_metadata()

        deserialized = Event.deserialize(raw_dict)

        assert dummy_event == deserialized

    def test_deserialize_no_event_type(self):
        with self.assertRaises(Exception):
            Event.deserialize({})

    def test_deserialize_event_type_does_not_exist(self):
        with self.assertRaises(Exception):
            Event.deserialize({"event_type": "__this_event_type_does_not_exist"})

    def test_network_propagate(self):
        new_event = FakeA().init({"key": 1})
        assert not new_event._network_propagate
        new_event_instance = new_event.network_propagate()
        assert new_event._network_propagate
        assert new_event_instance == new_event # verify functional programming works


    def test_str(self):
        
        fake_a = FakeA()
        fake_a_str = str(fake_a)

        assert FakeA.__name__ in fake_a_str



class FakeA(Event):
    def __init__(self):
        super().__init__(priority=10, schema=Schema({"key": int}))

class FakeB(Event):
    def __init__(self):
        super().__init__(priority=10, schema=Schema({}))

class LowPriority(Event):
    def __init__(self):
        super().__init__(priority=100, schema=Schema({}))

class MediumPriority(Event):
    def __init__(self):
        super().__init__(priority=50, schema=Schema({}))

class HighPriority(Event):
    def __init__(self):
        super().__init__(priority=0, schema=Schema({}))


class LowPriority(Event):
    def __init__(self):
        super().__init__(priority=100, schema=Schema({}))

class DummyEvent(Event):
    def __init__(self):
        super().__init__(priority=10, schema=Schema({
            "prop_1": int,
            "prop_2": And(str, len)
        }))
