import asyncio
import unittest
from unittest.mock import patch, MagicMock, PropertyMock
from tortoise.contrib.test import initializer, finalizer
from collections import namedtuple

from msa.builtins.scripting.handlers import AddScriptHandler, TriggerScriptRunHandler
from msa.builtins.scripting.events import (
    AddScriptEvent,
    TriggerScriptRunEvent,
    AddScriptFailedEvent,
    RunScriptResultEvent,
)
from msa.builtins.scripting import entities
from msa import core as msa_core

from tests.async_test_util import AsyncMock

FakeScriptEntity = namedtuple("ScriptEntitiy", ["name", "script_contents", "crontab"])


class AddScriptHandlerTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.create")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    def test_adding_script_no_crontab(
        self,
        ScriptEntity_FilterMock,
        ScriptEntityMock_CreateMock,
        ScriptExecutionManagerMock,
    ):

        first_mock = AsyncMock(return_value=None)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_FilterMock.return_value = result_mock

        ScriptEntityMock_CreateMock.return_value = identity_coro()

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()

        ScriptExecutionManagerMock.shared_state = (
            {}
        )  # for some reason this only works as a context manager
        handler = AddScriptHandler(loop, event_bus_mock, logger_mock)

        event_data = {"name": "test_script", "script_contents": 'print("hello world")'}
        add_script_event = AddScriptEvent().init(event_data)

        loop.run_until_complete(handler.handle_add_script_event(add_script_event))

        create_kwargs = ScriptEntityMock_CreateMock.call_args_list[0][1]

        expected = {"crontab": None, **event_data}
        self.assertEqual(expected, create_kwargs)

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.create")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    def test_adding_script_with_crontab(
        self,
        ScriptEntity_FilterMock,
        ScriptEntityMock_CreateMock,
        ScriptExecutionManagerMock,
    ):

        first_mock = AsyncMock(return_value=None)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_FilterMock.return_value = result_mock

        ScriptEntityMock_CreateMock.return_value = identity_coro()

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": MagicMock(),
        }
        handler = AddScriptHandler(loop, event_bus_mock, logger_mock)

        event_data = {
            "name": "test_script",
            "script_contents": 'print("hello world")',
            "crontab": "5 4 * * *",
        }
        add_script_event = AddScriptEvent().init(event_data)

        loop.run_until_complete(handler.handle_add_script_event(add_script_event))

        create_kwargs = ScriptEntityMock_CreateMock.call_args_list[0][1]

        expected = {"crontab": None, **event_data}
        self.assertEqual(expected, create_kwargs)

        scheduled_script = ScriptExecutionManagerMock.shared_state[
            "scheduled_scripts"
        ].get("test_script", None)

        self.assertIsNotNone(scheduled_script)
        self.assertIsNotNone(scheduled_script.get("task"))
        self.assertIsNotNone(scheduled_script.get("aiocron_instance"))

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_adding_script_with_name_that_already_exists(
        self, SupervisorMock, ScriptEntity_FilterMock, ScriptExecutionManagerMock
    ):

        first_mock = AsyncMock(return_value=FakeScriptEntity("a", "b", "c"))
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_FilterMock.return_value = result_mock

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": MagicMock(),
        }
        handler = AddScriptHandler(loop, event_bus_mock, logger_mock)

        event_data = {
            "name": "test_script",
            "script_contents": 'print("hello world")',
            "crontab": "5 4 * * *",
        }
        add_script_event = AddScriptEvent().init(event_data)

        loop.run_until_complete(handler.handle_add_script_event(add_script_event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]

        self.assertIsInstance(fired_event, AddScriptFailedEvent)
        self.assertEqual(fired_event.data["error"], "Failed to add script test_script.")
        self.assertEqual(
            fired_event.data["description"], "A script with this name already exists."
        )


class TriggerScriptRunHandlerTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    def test_ensure_successful_trigger_of_existing_script(
        self, ScriptEntity_FilterMock, ScriptExecutionManagerMock
    ):
        script_name = "test_script"
        script_contents = """print("hello")"""
        crontab = None
        event_data = {"name": script_name}

        script_entity = FakeScriptEntity(script_name, script_contents, crontab)

        first_mock = AsyncMock(return_value=script_entity)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_FilterMock.return_value = result_mock

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": MagicMock(),
        }
        handler = TriggerScriptRunHandler(loop, event_bus_mock, logger_mock)
        handler.script_execution_manager = ScriptExecutionManagerMock

        event = TriggerScriptRunEvent().init(event_data)
        loop.run_until_complete(handler.handle_trigger_script_run_event(event))

        call_args = ScriptExecutionManagerMock.schedule_script.call_args_list[0][0]
        expected = (script_name, script_contents, None)

        self.assertEqual(call_args, expected)

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_ensure_report_script_does_not_exist(
        self, SupervisorMock, ScriptEntity_FilterMock, ScriptExecutionManagerMock
    ):
        script_name = "test_script"
        event_data = {"name": script_name}

        first_mock = AsyncMock(return_value=None)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_FilterMock.return_value = result_mock

        loop = asyncio.get_event_loop()
        event_bus_mock = MagicMock()
        logger_mock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": MagicMock(),
        }
        handler = TriggerScriptRunHandler(loop, event_bus_mock, logger_mock)
        handler.script_execution_manager = ScriptExecutionManagerMock

        event = TriggerScriptRunEvent().init(event_data)

        loop.run_until_complete(handler.handle_trigger_script_run_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]

        self.assertIsInstance(fired_event, RunScriptResultEvent)
        self.assertIn("Attempted to trigger script run", fired_event.data["error"])


async def identity_coro(return_value=None):
    return return_value
