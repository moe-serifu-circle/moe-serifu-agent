import asyncio
import unittest
from unittest.mock import patch, MagicMock, PropertyMock
from tortoise.contrib.test import initializer, finalizer
from collections import namedtuple
import aiocron


from msa.builtins.scripting.handlers import *
from msa.builtins.scripting.events import (
    AddScriptEvent,
    TriggerScriptRunEvent,
    AddScriptFailedEvent,
    RunScriptResultEvent,
)
from msa.builtins.signals.events import StartupEvent
from msa.builtins.scripting import entities
from msa import core as msa_core

from tests.async_test_util import AsyncMock

FakeScriptEntity = namedtuple("ScriptEntitiy", ["name", "script_contents", "crontab"])
FullFakeScriptEntity = namedtuple(
    "ScriptEntitiy",
    ["id", "name", "script_contents", "crontab", "created", "last_edited", "last_run"],
)


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


class StartupEventHandlerTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.all", new_callable=AsyncMock)
    def test_schedule_script_with_crontab(
        self, script_entity_all_mock, ScriptExecutionManagerMock
    ):

        test_script = FakeScriptEntity(
            "test_script", """print("hello world")""", "5 4 * * *"
        )
        test_scripts = [test_script]

        script_entity_all_mock.mock.return_value = test_scripts

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": loopMock,
        }

        handler = StartupEventHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event_data = {"timestamp": "fake timestamp"}
        event = StartupEvent().init(event_data)

        loop.run_until_complete(handler.handle_startup_event(event))

        schedule_script_args = ScriptExecutionManagerMock.schedule_script.call_args_list[
            0
        ][
            0
        ]

        expected_args = (
            test_script.name,
            test_script.script_contents,
            test_script.crontab,
        )
        self.assertEqual(schedule_script_args, expected_args)


class TriggerScriptListHandlerTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.all", new_callable=AsyncMock)
    @patch("msa.core.supervisor_instance")
    def test_list_scripts(
        self, SupervisorMock, script_entity_all_mock, ScriptExecutionManagerMock
    ):
        test_script1 = FullFakeScriptEntity(
            1,
            "test_script1",
            """print("hello world")""",
            "5 4 * * *",
            datetime.now(),
            datetime.now(),
            datetime.now(),
        )
        test_script2 = FullFakeScriptEntity(
            2,
            "test_script2",
            """print("hello world")""",
            None,
            datetime.now(),
            datetime.now(),
            None,
        )
        test_scripts = [test_script1, test_script2]

        script_entity_all_mock.mock.return_value = test_scripts

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        aiocronMock = MagicMock()

        aiocronMock.handle.when.return_value = 5
        aiocronMock.time.return_value = datetime.now().timestamp()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {"test_script1": {"aiocron_instance": aiocronMock}},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerScriptListHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerListScriptsEvent()
        loop.run_until_complete(handler.handle_trigger_script_list_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.ListScriptsEvent)

        self.assertIn("scripts", fired_event.data)
        validatedScripts = []
        for script in fired_event.data["scripts"]:
            if script["id"] == 1:
                self.assertEqual(script["name"], test_script1.name)
                self.assertEqual(script["crontab"], test_script1.crontab)
                self.assertNotEqual(script["last_run"], None)
                self.assertFalse(script["running"])
                self.assertEqual(script["crontab"], test_script1.crontab)
                validatedScripts.append(1)

            elif script["id"] == 2:
                self.assertEqual(script["name"], test_script2.name)
                self.assertEqual(script["crontab"], test_script2.crontab)
                self.assertEqual(script["last_run"], None)
                self.assertFalse(script["running"])
                validatedScripts.append(2)

        self.assertEqual(validatedScripts, [1, 2])


class TriggerGetScripHandlertTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_get_script_with_crontab(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):
        test_script = FullFakeScriptEntity(
            1,
            "test_script1",
            """print("hello world")""",
            "5 4 * * *",
            datetime.now(),
            datetime.now(),
            datetime.now(),
        )

        first_mock = AsyncMock(return_value=test_script)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        aiocronMock = MagicMock()

        aiocronMock.handle.when.return_value = 5
        aiocronMock.time.return_value = datetime.now().timestamp()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {"test_script1": {"aiocron_instance": aiocronMock}},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerGetScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": test_script.name})
        loop.run_until_complete(handler.handle_trigger_get_script_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.GetScriptEvent)

        self.assertEqual(fired_event.data["id"], test_script.id)
        self.assertEqual(fired_event.data["name"], test_script.name)
        self.assertEqual(fired_event.data["crontab"], test_script.crontab)
        self.assertEqual(
            fired_event.data["script_contents"], test_script.script_contents
        )
        self.assertFalse(fired_event.data["running"])

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_get_script_without_crontab(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):
        test_script = FullFakeScriptEntity(
            1,
            "test_script1",
            """print("hello world")""",
            None,
            datetime.now(),
            datetime.now(),
            datetime.now(),
        )

        first_mock = AsyncMock(return_value=test_script)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        aiocronMock = MagicMock()

        aiocronMock.handle.when.return_value = 5
        aiocronMock.time.return_value = datetime.now().timestamp()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerGetScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": test_script.name})
        loop.run_until_complete(handler.handle_trigger_get_script_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.GetScriptEvent)

        self.assertEqual(fired_event.data["id"], test_script.id)
        self.assertEqual(fired_event.data["name"], test_script.name)
        self.assertEqual(fired_event.data["crontab"], test_script.crontab)
        self.assertEqual(
            fired_event.data["script_contents"], test_script.script_contents
        )
        self.assertFalse(fired_event.data["running"])


class TriggerDeleteScriptHandlerTest(unittest.TestCase):
    def setUp(self):
        initializer(
            ["msa.builtins.scripting.entities"], db_url="sqlite:///tmp/test-{}.sqlite"
        )

    def tearDown(self) -> None:
        finalizer()

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_delete_running_script_cancel_successfully(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):
        test_script = MagicMock()
        test_script.name = "test_script"
        test_script.script_contents = """print("hello world")"""
        test_script.crontab = "5 4 * * *"
        test_script.delete = AsyncMock()

        first_mock = AsyncMock(return_value=test_script)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        aiocronMock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {
                test_script.name: {"aiocron_instance": aiocronMock, "task": None}
            },
            "loop": loopMock,
            "running_scripts": set([test_script.name]),
        }

        handler = TriggerDeleteScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": test_script.name})

        async def runner():
            ScriptExecutionManagerMock.shared_state["scheduled_scripts"][
                test_script.name
            ]["task"] = asyncio.create_task(identity_coro())
            await handler.handle_trigger_delete_script_event(event)

        loop.run_until_complete(runner())

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.ScriptDeletedEvent)

        self.assertEqual(fired_event.data["status"], "success")
        self.assertEqual(fired_event.data["name"], test_script.name)

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_delete_not_running_script(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):
        test_script = MagicMock()
        test_script.name = "test_script"
        test_script.script_contents = """print("hello world")"""
        test_script.crontab = "5 4 * * *"
        test_script.delete = AsyncMock()

        first_mock = AsyncMock(return_value=test_script)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerDeleteScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": test_script.name})

        loop.run_until_complete(handler.handle_trigger_delete_script_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.ScriptDeletedEvent)

        self.assertEqual(fired_event.data["status"], "success")
        self.assertEqual(fired_event.data["name"], test_script.name)

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_fail_to_delete_script(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):
        test_script = MagicMock()
        test_script.name = "test_script"
        test_script.script_contents = """print("hello world")"""
        test_script.crontab = "5 4 * * *"
        test_script.delete = AsyncMock()
        test_script.delete.mock.side_effect = Exception("fail to delete")

        first_mock = AsyncMock(return_value=test_script)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerDeleteScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": test_script.name})

        loop.run_until_complete(handler.handle_trigger_delete_script_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.ScriptDeletedEvent)

        self.assertEqual(fired_event.data["status"], "failure")
        self.assertEqual(fired_event.data["name"], test_script.name)
        self.assertEqual(
            fired_event.data["reason"], "Failed to delete script with name test_script."
        )

    @patch("msa.builtins.scripting.script_execution_manager.ScriptExecutionManager")
    @patch("msa.builtins.scripting.entities.ScriptEntity.filter")
    @patch("msa.core.supervisor_instance")
    def test_fail_to_delete_cannot_find_script(
        self, SupervisorMock, ScriptEntity_filter_mock, ScriptExecutionManagerMock
    ):

        first_mock = AsyncMock(return_value=None)
        result_mock = MagicMock()
        result_mock.first = first_mock
        ScriptEntity_filter_mock.return_value = result_mock

        loop = asyncio.get_event_loop()
        loopMock = MagicMock()
        eventBusMock = MagicMock()
        loggerMock = MagicMock()

        ScriptExecutionManagerMock.shared_state = {
            "scheduled_scripts": {},
            "loop": loopMock,
            "running_scripts": set(),
        }

        handler = TriggerDeleteScriptHandler(loopMock, eventBusMock, loggerMock)
        handler.script_manager = ScriptExecutionManagerMock

        event = events.TriggerGetScriptEvent().init({"name": "test_script"})

        loop.run_until_complete(handler.handle_trigger_delete_script_event(event))

        fired_event = SupervisorMock.fire_event.call_args_list[0][0][0]
        self.assertIsInstance(fired_event, events.ScriptDeletedEvent)

        self.assertEqual(fired_event.data["status"], "failure")
        self.assertEqual(fired_event.data["name"], "test_script")
        self.assertEqual(
            fired_event.data["reason"], "Unable to find script with name test_script."
        )


async def identity_coro(return_value=None):
    return return_value
