#ifndef MSA_EVENT_DISPATCH_HPP
#define MSA_EVENT_DISPATCH_HPP

#include "msa.hpp"
#include "event/handler.hpp"
#include "event/timer.hpp"
#include "cfg/cfg.hpp"

namespace msa { namespace event {

	extern int init(msa::Handle msa, const msa::cfg::Section &config);
	extern int quit(msa::Handle msa);
	extern int setup(msa::Handle hdl);	
	extern int teardown(msa::Handle hdl);
	extern const PluginHooks *get_plugin_hooks();

	/**
	 * This method should only be called from the timer system!
	 */
	extern *TimerContext get_timer_context(msa::Handle hdl);
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "event/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "event/hooks.hpp"
		#undef MSA_MODULE_HOOK
		TimerHooks timer;
	};
} }

#endif
