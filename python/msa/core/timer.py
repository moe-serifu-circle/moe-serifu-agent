"""
Operates system and user timers. This is used for managing repetitive tasks that are set to start in the future. Every
timer emits a particular event after a certain amount of time, and may repeat that event if scheduled to be recurring.

The Timer class is used for tracking the timing of a particular task, but this class is not intended for use on its own.
Instead, a TimerManager instance is used for maintaining each timer as well as for managing variables that apply to all
timers.
"""

class Timer(object):
	"""
	Holds a single CLI invocation for future execution. Keeps the time that the command is scheduled for execution as
	well as whether it is to be repeated.
	"""


	def __init__(self, period_ms, ):
		"""
		Create a new Timer fo
		:param tick_resolution:
		"""