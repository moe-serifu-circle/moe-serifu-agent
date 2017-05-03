#include "event/timer.hpp"

#include "event/handler.hpp"
#include "event/dispatch.hpp"
#include "log/log.hpp"
#include "agent/agent.hpp"

#include <map>

#include "platform/thread/thread.hpp"


namespace msa { namespace event {
	
	typedef std::chrono::high_resolution_clock chrono_clock;
	typedef chrono_clock::time_point chrono_time;
	
	class Timer
	{
		public:
			Timer(int16_t id, std::chrono::milliseconds period, Topic topic, const IArgs &args, bool recurring) :
				_id(id),
				_period(period),
				_last_fired(chrono_clock::now()),
				_recurring(recurring),
				_event_args(args.copy()),
				_event_topic(topic)
			{}
			
			Timer(const Timer &other) :
				_id(other._id),
				_period(other._period),
				_last_fired(other._last_fired),
				_recurring(other._recurring),
				_event_args(other._event_args->copy()),
				_event_topic(other._event_topic)
			{}
			
			~Timer()
			{
				delete _event_args;
			}
			
			Timer &operator=(const Timer &other)
			{
				delete event_args;
				_event_args = other._event_args->copy();
				_id = other._id;
				_period = other._period;
				_last_fired = other._last_fired;
				_event_topic = other._event_topic;
				return *this;
			}
			
			/**
			 * Check if the timer is ready to fire at the current time.
			 *
			 * Now time is given rather than calculated so that missing
			 * a fire time does not cause schedule slip.
			 */
			bool ready(chrono_time now) const
			{
				return _last_fired + _period <= now;
			}

			/**
			 * Fires the timer, causing its event to be generated.
			 *
			 * Now time is given rather than calculated so that missing
			 * a fire time does not cause schedule slip.
			 */
			void fire(msa::Handle hdl, chrono_time now)
			{
				generate(hdl, _event_topic, *_event_args);
				msa::log::debug(hdl, "Fired timer " + std::to_string(id()));
				_last_fired = now;
			}
			
			bool recurring() const
			{
				return _recurring;
			}
			
			int16_t id() const
			{
				return _id;
			}
			
			Topic topic() const
			{
				return _event_topic;
			}

		private:
			int16_t _id;
			std::chrono::milliseconds _period;
			std::chrono::high_resolution_clock::time_point _last_fired;
			bool _recurring;
			IArgs *_event_args;
			Topic _event_topic;
	};
	
	struct TimerContext
	{
		std::chrono::milliseconds tick_resolution;
		chrono_time last_tick_time;
		std::map<int16_t, Timer*> timers;
		msa::thread::Mutex mutex;
	};
	
	static void fire_timers(msa::Handle hdl, chrono_time now)
	static void cmd_timer(msa::Handle hdl, const msa::cmd::ParamList &params, HandlerSync *const sync);
	static void cmd_deltimer(msa::Handle hdl, const msa::cmd::ParamList &params, HandlerSync *const sync);

	extern std::vector<msa::cmd::Command *> get_timer_commands()
	{
		std::vector<msa::cmd::Command *> cmds;
		cmds.push_back(new msa::cmd::Command("TIMER", "It schedules a command to execute in the future", "time-ms command", "r", cmd_timer));
		cmds.push_back(new msa::cmd::Command("DELTIMER", "It deletes a timer", "timer-id", cmd_deltimer));
		return cmds;
	}

	extern int16_t schedule(msa::Handle msa, time_t timestamp, const Topic topic, const IArgs &args)
	{
		time_t ref_time = time(NULL);
		if (ref_time >= timestamp)
		{
			return -1;
		}
		return delay(msa, std::chrono::seconds(timestamp), topic, args);
	}

	extern int16_t delay(msa::Handle msa, std::chrono::milliseconds delay, const Topic topic, const IArgs &args)
	{
		msa::thread::mutex_lock(&msa->timer->mutex);
		int16_t id = msa->timer->list.size();
		Timer *t = new Timer(id, delay, topic, args, false);
		msa->timer->list[t->id] = t;
		msa::thread::mutex_unlock(&msa->timer->mutex);
		msa::log::debug(msa, "Scheduled a " + topic_str(topic) + " event to fire in " + std::to_string(delay.count()) + "ms (id = " + std::to_string(t->id()) + ")");
		return t->id;
	}
	
	extern int16_t add_timer(msa::Handle msa, std::chrono::milliseconds period, const Topic topic, const IArgs &args)
	{
		msa::thread::mutex_lock(&msa->timer->mutex);
		int16_t id = msa->timer->list.size();
		Timer *t = new Timer(period, topic, args, true);
		msa->timer->list[t->id] = t;
		msa::thread::mutex_unlock(&msa->timer->mutex);
		msa::log::debug(msa, "Scheduled a " + topic_str(topic) + " event to fire every " + std::to_string(period.count()) + "ms (id = " + std::to_string(t->id()) + ")");
		return t->id;
	}

	extern void remove_timer(msa::Handle msa, int16_t id)
	{
		TimerContext *ctx = msa->timer;
		msa::thread::mutex_lock(&ctx->mutex);
		if (ctx->list.find(id) == ctx->list.end())
		{
			msa::thread::mutex_unlock(&ctx->mutex);
			throw std::logic_error("no timer with ID: " + std::to_string(id));
		}
		Timer *t = ctx->list[id];
		ctx->list.erase(id);
		msa::thread::mutex_unlock(&ctx->mutex);
		delete t;
		msa::log::debug(msa, "Removed timer ID " + std::to_string(id));
		return;
	}

	extern void get_timers(msa::Handle msa, std::vector<int16_t> &list)
	{
		TimerContext *ctx = msa->timer;
		msa::thread::mutex_lock(&ctx->mutex);
		std::map<int16_t, Timer*>::const_iterator iter;
		for (iter = ctx->list.begin(); iter != ctx->list.end(); iter++)
		{
			list.push_back(iter->first);
		}
		msa::thread::mutex_unlock(&ctx->mutex);
	}
	
	extern int create_timer_context(TimerContext **ctx)
	{	
		TimerContext *t = new TimerContext;
		t->last_tick_time = chrono_time::min();
		msa::thread::mutex_init(&t->mutex, NULL);
		t->tick_resolution = 1;
		*ctx = t;
		return 0;
	}
	
	extern void dispose_timer_context(TimerContext *ctx)
	{	
		msa::thread::mutex_destroy(ctx->mutex);
		delete ctx;
	}
	
	extern void set_tick_resolution(TimerContext *ctx, int res)
	{
		ctx->tick_resolution = std::chrono::milliseconds(res);
	}
	
	extern void clear_timers(TimerContext *ctx)
	{
		auto timer_iter = ctx->list.begin();
		while (timer_iter != ctx->list.end())
		{
			Timer *t = timer_iter->second;
			timer_iter = ctx->list.erase(timer_iter);
			delete t;
		}
	}
	
	extern void check_timers(msa::Handle hdl)
	{
		TimerContext *ctx = hdl->timer;
		
		// check if we need to do timing tasks
		chrono_time now = chrono_clock::now();
		if (ctx->last_tick_time + ctx->tick_resolution <= now)
		{
			ctx->last_tick_time = now;
			fire_timers(hdl, now);
		}
	}

	static void fire_timers(msa::Handle hdl, chrono_time now)
	{
		TimerContext *ctx = hdl->timer;
		msa::thread::mutex_lock(&ctx->mutex);
		std::map<int16_t, Timer*>::iterator iter = ctx->list.begin();
		while (iter != ctx->list.end())
		{
			Timer *t = iter->second;
			if (t->ready(now))
			{
				t->fire(now);
				if (!t->recurring())
				{
					iter = ctx->list.erase(iter);
					delete t;
					msa::log::debug(hdl, "Completed and removed timer " + std::to_string(iter->first));
					continue;
				}
			}
			iter++;
		}
		msa::thread::mutex_unlock(&ctx->mutex);
	}
	
	static void cmd_timer(msa::Handle hdl, const msa::cmd::ParamList &params, HandlerSync *const UNUSED(sync))
	{
		bool recurring = params.has_option('r');
		if (params.arg_count() < 2)
		{
			msa::agent::say(hdl, "You gotta give me a time and a command to execute, $USER_TITLE.");
			return;
		}
		int period = 0;
		try
		{
			period = std::stoi(params[0]);
		}
		catch (std::exception &e)
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but '" + params[0] + "' isn't a number of milliseconds.");
			return;
		}
		if (period < 0)
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, I might be good but I can't go back in time.");
			msa::agent::say(hdl, "Please give me a positive number of milliseconds.");
			return;
		}
		auto ms = std::chrono::milliseconds(period);
		std::string cmd_str = "";
		for (size_t i = 1; i < params.arg_count(); i++)
		{
			cmd_str += params[i];
			if (i + 1 < params.arg_count())
			{
				cmd_str += " ";
			}
		}
		std::string plural = ms.count() != 1 ? "s" : "";
		std::string type = recurring ? "every" : "in";
		int16_t id = -1;
		if (recurring)
		{
			id = add_timer(hdl, ms, Topic::TEXT_INPUT, wrap(cmd_str));
		}
		else
		{
			id = delay(hdl, ms, Topic::TEXT_INPUT, wrap(cmd_str));
		}
		if (id == -1)
		{
			msa::agent::say(hdl, "Oh no! I'm sorry, $USER_TITLE, that didn't work quite right!");
		}
		else
		{
			msa::agent::say(hdl, "Okay, $USER_TITLE, I will do that " + type + " " + std::to_string(ms.count()) + " millisecond" + plural + "!");
			msa::agent::say(hdl, "The timer ID is " + std::to_string(id) + ".");
		}
	}	

	static void cmd_deltimer(msa::Handle hdl, const msa::cmd::ParamList &params, HandlerSync *const UNUSED(sync))
	{
		if (params.arg_count() < 1)
		{
			msa::agent::say(hdl, "Ahh... $USER_TITLE, I need to know which timer I should delete.");
			return;
		}
		int16_t id = 0;
		try
		{
			id = std::stoi(params[0]);
		}
		catch (std::exception &e)
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but '" + params[0] + "' isn't an integer.");
			return;
		}
		remove_timer(hdl, id);
		msa::agent::say(hdl, "Okay! I stopped timer " + std::to_string(id) + " for you, $USER_TITLE.");
	}

} }
