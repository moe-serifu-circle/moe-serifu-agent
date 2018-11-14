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

class DummyEvent(Event):
    def __init__(self):
        super().__init__(priority=10, schema=Schema({
            "prop_1": int,
            "prop_2": And(str, len)
        }))
