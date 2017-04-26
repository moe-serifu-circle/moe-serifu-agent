#ifndef MSA_EVENT_DISPATCH_HPP
#define MSA_EVENT_DISPATCH_HPP

#include "msa.hpp"
#include "event/handler.hpp"
#include "cfg/cfg.hpp"

#include <chrono>

namespace msa { namespace event {

	typedef struct timer_type
	{
		int16_t id;
		std::chrono::milliseconds period;
		std::chrono::high_resolution_clock::time_point last_fired;
		bool recurring;
		void *event_args;
		Topic event_topic;
		size_t event_args_size;
	} Timer;

	extern int init(msa::Handle msa, const msa::cfg::Section &config);
	extern int quit(msa::Handle msa);
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "event/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "event/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
} }

#endif
