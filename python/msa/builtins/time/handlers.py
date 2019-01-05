from msa.core.event import Event

import asyncio
import time
import logging
import math

from msa.core.event_handler import EventHandler
from msa.core import supervisor
from msa.builtins.time import timer

from msa.builtins.time.events import TimeEvent, DelTimerCommandEvent, TimerCommandEvent
from msa.builtins.command_registry.events import RegisterCommandEvent
from msa.builtins.tty.events import TextOutputEvent, TextInputEvent


_log = logging.getLogger(__name__)


class TimeHandler(EventHandler):
    """
    Fires a TimeEvent at the beginning of every minute
    """

    def __init__(self, loop, event_queue, logger, config=None):
        super().__init__(loop, event_queue, logger, config)

        self.timer_manager = timer.TimerManager()
        tick_res = 10
        if config is not None:
            if 'tick_resolution' in config:
                tick_res = int(config['tick_resolution'])

        # TODO: merge with ctor
        self.timer_manager.tick_resolution = tick_res

    async def init(self):
        # TODO: strictly speaking, this is a type, not a constructor (although it is callable)
        timer_data = {
            'event_constructor': TimerCommandEvent,
            'invoke': 'timer',
            'describe': 'Creates a new timer event',
            'usage': 'timer <-r> [command...]'
        }
        deltimer_data = {
            'event_constructor': DelTimerCommandEvent,
            'invoke': 'deltimer',
            'describe': "Removes a timer by ID",
            'usage': 'deltimer [id]'
        }

        register_timer_event = RegisterCommandEvent()
        register_timer_event.init(timer_data)

        register_deltimer_event = RegisterCommandEvent()
        register_deltimer_event.init(deltimer_data)

        supervisor.fire_event(register_timer_event)
        supervisor.fire_event(register_deltimer_event)

    async def handle(self):
        e = None
        try:
            _, e = self.event_queue.get_nowait()
        except asyncio.QueueEmpty:
            # this is fine; we just don't want to wait
            pass

        if e is not None:
            asyncio.ensure_future(self._handle_event(e), loop=self.loop)

        delta = await self._handle_timer()
        await asyncio.sleep(delta)

    async def _handle_timer(self):
        # TODO: ensure we aren't skipping timer fire checks too frequently
        start = time.monotonic()  # get now to determine when to wake up

        new_event = TimeEvent()
        new_event.init({
            "current_time": start
        })

        supervisor.fire_event(new_event)

        await self.timer_manager.check_timers()
        # check them again in 50ms - no gaurentee this will happen, but hopefully scheduled frequently enough to not
        # matter. Definitely a monkey patch for now
        wait_time = 0.05
        delta = wait_time - (time.monotonic() - start)
        if delta < 0:
            # we have missed a fire due to firing taking too long. No problem, just skip as many rounds as have been
            # missed

            # calculate wait time until next firing point
            delta = wait_time + math.modf(delta)[0]
        return delta

    async def _handle_event(self, event: Event):
        if isinstance(event, DelTimerCommandEvent):
            args = event.data['raw_text'].split()[1:]
            await self._execute_del_timer_command(args)
        elif isinstance(event, TimerCommandEvent):
            args = event.data['raw_text'].split()[1:]
            await self._execute_timer_command(args)

    async def _execute_timer_command(self, args):
        """
        Command to add a timer

        :param args: Args to this command. Must contain one element, an int ID.
        :return: Exit status of this command.
        """
        _log.debug("Args: " + repr(args))

        # TODO: better way of parsing command options
        recurring = '-r' in args
        if recurring:
            args.remove('-r')

        if len(args) < 2:
            e = TextOutputEvent()
            e.init({'message': "You gotta give me a time and a command to execute, $USER_TITLE.\n"})
            supervisor.fire_event(e)
            return 1

        try:
            period = int(args[0])
        except ValueError:
            e = TextOutputEvent()
            e.init({'message': "Sorry, $USER_TITLE, but " + repr(args[0]) + " isn't a number of milliseconds.\n"})
            supervisor.fire_event(e)
            return 2

        if period < 0:
            e = TextOutputEvent()
            e.init({'message': "Sorry, $USER_TITLE, I might be good but I can't go back in time.\n"})
            supervisor.fire_event(e)
            e = TextOutputEvent()
            e.init({'message': "Please give me a positive number of milliseconds.\n"})
            supervisor.fire_event(e)
            return 3

        cmd_str = ""
        for tok in args[1:]:
            cmd_str += tok + ' '
        cmd_str = cmd_str[:-1]

        plural = 's' if period != 1 else ''
        rec_type = 'every' if recurring else 'in'

        # noinspection PyBroadException
        try:
            if recurring:
                id = await self.timer_manager.add_timer(period, TextInputEvent, {'message': cmd_str})
            else:
                id = await self.timer_manager.delay(period, TextInputEvent, {'message': cmd_str})
        except Exception:
            _log.exception('problem scheduling timer')
            e = TextOutputEvent()
            e.init({'message': "Oh no! I'm sorry, $USER_TITLE, that didn't work quite right!\n"})
            supervisor.fire_event(e)
            return 4

        e = TextOutputEvent()
        text = "Okay, $USER_TITLE, I will do that {:s} {:d} millisecond{:s}!\n".format(rec_type, period, plural)
        e.init({'message': text})
        supervisor.fire_event(e)

        e = TextOutputEvent()
        e.init({'message': "The timer ID is {:d}.\n".format(id)})
        supervisor.fire_event(e)
        return 0

    async def _execute_del_timer_command(self, args):
        """
        Command to remove a timer

        :param args: Args to this command. Must contain one element, an int ID.
        :return: Exit status of this command.
        """
        if len(args) < 1:
            e = TextOutputEvent()
            e.init({'message': "Ahh... $USER_TITLE, I need to know which timer I should delete.\n'"})
            supervisor.fire_event(e)
            return 1
        try:
            id = int(args[0])
        except ValueError:
            e = TextOutputEvent()
            e.init({'message': "Sorry, $USER_TITLE, but " + repr(args[0]) + " isn't an integer.\n"})
            supervisor.fire_event(e)
            return 2
        await self.timer_manager.remove_timer(id)

        e = TextOutputEvent()
        e.init({'message': "Okay! I stopped timer " + str(id) + " for you, $USER_TITLE.\n"})
        supervisor.fire_event(e)

        return 0
