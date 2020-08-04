import asyncio
import unittest
from unittest.mock import patch, MagicMock, PropertyMock
from tortoise.contrib.test import initializer, finalizer

from msa.builtins.scripting.handlers import AddScriptHandler
from msa.builtins.scripting.events import AddScriptEvent
from msa.builtins.scripting import entities

from tests.async_test_util import AsyncMock


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

        # TODO: caputre supervisor.fire_event input

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


async def identity_coro(return_value=None):
    return return_value
