#ifndef MSA_EVENT_TIMER_HPP
#define MSA_EVENT_TIMER_HPP

#include "msa.hpp"
#include "cmd/cmd.hpp"
#include "event/event.hpp"

#include <chrono>
#include <vector>

namespace msa { namespace event {

	extern std::vector<msa::cmd::Command *> get_timer_commands();
	extern void check_timers(msa::Handle hdl);
	extern int create_timer_context(TimerContext **ctx);
	extern void dispose_timer_context(TimerContext *ctx);
	extern void set_tick_resolution(TimerContext *ctx, int res);
	extern void clear_timers(TimerContext *ctx);
	extern void sys_remove_timer(msa::Handle msa, int16_t id);
	extern int16_t sys_add_timer(msa::Handle msa, std::chrono::milliseconds period, const Topic topic, const IArgs &args);

	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "event/timer_hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct TimerHooks
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "event/timer_hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
} }

#endif
