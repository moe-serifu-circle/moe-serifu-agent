#ifndef MSA_EVENT_TIMER_HPP
#define MSA_EVENT_TIMER_HPP

#include "msa.hpp"
#include "cmd/cmd.hpp"

#include <chrono>
#include <vector>

namespace msa { namespace event {
	
	struct TimerContext
	{
		std::chrono::milliseconds tick_resolution;
		chrono_time last_tick_time;
		std::map<int16_t, Timer*> list;
		msa::thread::Mutex mutex;
	};

	extern std::vector<msa::cmd::Command *> get_timer_commands();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "event/timer_hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct TimerHooks
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "event/timer_hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
	
	typedef std::chrono::high_resolution_clock chrono_clock;
	typedef chrono_clock::time_point chrono_time;
	
	class Timer
	{
		public:
			Timer(std::chrono::milliseconds period, Topic topic, const IArgs &args, bool recurring) :
				id(0),
				period(period),
				last_fired(chrono_clock::now()),
				recurring(recurring),
				event_args(args.copy()),
				event_topic(topic)
			{}
			Timer(const Timer &other) :
				id(other.id),
				period(other.period),
				last_fired(other.last_fired),
				recurring(other.recurring),
				event_args(other.event_args->copy()),
				event_topic(other.event_topic)
			{}
			~Timer()
			{
				delete event_args;
			}
			Timer &operator=(const Timer &other)
			{
				delete event_args;
				event_args = other.event_args->copy();
				id = other.id;
				period = other.period;
				last_fired = other.last_fired;
				event_topic = other.event_topic;
				return *this;
			}
			
			/**
			 * Check if the timer is ready to fire at the current time.
			 *
			 * Now time is given rather than calculated so that missing
			 * a fire time does not cause schedule slip.
			 */
			bool ready(chrono_time now) const;

			/**
			 * Fires the timer, causing its event to be generated.
			 *
			 * Now time is given rather than calculated so that missing
			 * a fire time does not cause schedule slip.
			 */
			void fire(msa::Handle hdl, chrono_time now);

		private:
			int16_t _id;
			const std::chrono::milliseconds _period;
			std::chrono::high_resolution_clock::time_point _last_fired;
			const bool _recurring;
			IArgs *const _event_args;
			const Topic _event_topic;
	};
} }

#endif
