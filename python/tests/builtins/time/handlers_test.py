import unittest
import logging
from unittest import mock
import asyncio

from tests.async_test_util import AsyncMock, AsyncMockWithArgs

from msa.core import LogicError, ProtectionError

from msa.builtins.time.events import DelTimerCommandEvent, TimerCommandEvent
from msa.builtins.tty.events import TextInputEvent
from msa.builtins.time.handlers import TimeHandler

import time


class TimeHandlerTests(unittest.TestCase):

    def setUp(self):
        self.loop = asyncio.new_event_loop()
        self.event_queue = asyncio.PriorityQueue(loop=self.loop)
        dummy_logger = logging.getLogger('foo')
        dummy_logger.addHandler(logging.NullHandler())
        self.handler = TimeHandler(loop=self.loop, event_queue=self.event_queue, logger=dummy_logger)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch('msa.builtins.time.TimeHandler._execute_deltimer_command', new_callable=AsyncMock)
    def test_deltimer_event_causes_deltimer_function(self, deltimer_coro):

        # Create command event
        raw_text = "deltimer 25"
        event = DelTimerCommandEvent()
        event.init({
            "raw_text": raw_text,
            "tokens": raw_text.split()[1:]
        })
        self.event_queue.put_nowait((10, event))  # ensure lower priority

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.stop()
        self.loop.close()

        deltimer_coro.mock.assert_called_once()

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch("msa.core.supervisor.should_stop", new=mock.MagicMock(side_effect=[False, True]))
    @mock.patch('msa.builtins.time.TimeHandler._execute_timer_command', new_callable=AsyncMock)
    def test_timer_event_causes_timer_function(self, timer_coro):

        # Create command event
        raw_text = "timer 1000 echo test"
        event = TimerCommandEvent()
        event.init({
            "raw_text": raw_text,
            "tokens": raw_text.split()[1:]
        })
        self.event_queue.put_nowait((10, event))  # ensure lower priority

        # add handle wrapper to execution loop
        self.loop.create_task(self.handler.handle())

        # stop the loop after 0.5 seconds
        self.loop.call_later(0.5, lambda: self.loop.stop())

        # begin running loop
        self.loop.run_forever()
        self.loop.stop()
        self.loop.close()

        timer_coro.mock.assert_called_once()

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.time.timer.TimerManager.remove_timer', new_callable=AsyncMock)
    def test_deltimer_command(self, remove_timer_coro):
        cmd_args = ['25']

        status = self._run_coro(self.handler._execute_deltimer_command(cmd_args))

        self.assertEqual(status, 0)
        remove_timer_coro.mock.assert_called_once_with(mock.ANY, 25)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_deltimer_command_missing_id(self):
        cmd_args = []

        status = self._run_coro(self.handler._execute_deltimer_command(cmd_args))

        self.assertEqual(status, 1)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_deltimer_command_unparsable_id(self):
        cmd_args = ['id']

        status = self._run_coro(self.handler._execute_deltimer_command(cmd_args))

        self.assertEqual(status, 2)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_deltimer_command_invalid_id(self):
        cmd_args = ['4']

        status = self._run_coro(self.handler._execute_deltimer_command(cmd_args))

        self.assertEqual(status, 3)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.time.timer.TimerManager.remove_timer', new_callable=AsyncMockWithArgs(side_effect=ProtectionError))
    def test_deltimer_command_system_timer(self, remove_timer_coro):
        cmd_args = ['4']

        status = self._run_coro(self.handler._execute_deltimer_command(cmd_args))

        self.assertEqual(status, 4)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.time.timer.TimerManager.delay', new_callable=AsyncMockWithArgs(return_value=1))
    def test_timer_command(self, delay_coro):
        cmd_args = ['1000', 'echo', 'test']

        status = self._run_coro(self.handler._execute_timer_command(cmd_args))

        self.assertEqual(status, 0)
        delay_coro.mock.assert_called_once_with(mock.ANY, 1000, TextInputEvent, {'message': 'echo test'})

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.time.timer.TimerManager.add_timer', new_callable=AsyncMockWithArgs(return_value=1))
    def test_timer_command_recurring(self, add_timer_coro):
        cmd_args = ['-r', '1000', 'echo', 'test']

        status = self._run_coro(self.handler._execute_timer_command(cmd_args))

        self.assertEqual(status, 0)
        add_timer_coro.mock.assert_called_once_with(mock.ANY, 1000, TextInputEvent, {'message': 'echo test'})

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_timer_command_missing_args(self):
        cases = [
            ['1000'],
            ['-r', '1000'],
            ['1000', '-r'],
            ['-r'],
            []
        ]

        for cmd_args in cases:
            status = self._run_coro(self.handler._execute_timer_command(cmd_args))

            self.assertEqual(status, 1, 'not detected as invalid case: ' + str(cmd_args))

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_timer_command_unparsable_period(self):
        cases = [
            ['', 'echo'],
            ['-', 'echo'],
            ['cat', 'echo']
        ]

        for cmd_args in cases:
            status = self._run_coro(self.handler._execute_timer_command(cmd_args))

            self.assertEqual(status, 2, 'not detected as invalid case: ' + str(cmd_args))

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    def test_timer_command_negative_period(self):
        cmd_args = ['-10', 'echo']

        status = self._run_coro(self.handler._execute_timer_command(cmd_args))

        self.assertEqual(status, 3)

    @mock.patch("msa.core.supervisor.fire_event", new=mock.Mock())
    @mock.patch('msa.builtins.time.timer.TimerManager.delay', new_callable=AsyncMockWithArgs(side_effect=RuntimeError))
    def test_timer_command_other_problem(self, delay_coro):
        cmd_args = ['1000', 'echo']

        status = self._run_coro(self.handler._execute_timer_command(cmd_args))

        self.assertEqual(status, 4)

    def _run_coro(self, func):
        result = -1

        async def wrapper():
            nonlocal result
            result = await func

        self.loop.run_until_complete(wrapper())
        return result


if __name__ == "__main__":
    unittest.main()
