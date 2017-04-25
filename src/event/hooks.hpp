// Functions in the event module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from event module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include event hooks before MSA_MODULE_HOOK macro is defined"
#endif

MSA_MODULE_HOOK(void, subscribe, msa::Handle msa, Topic topic, EventHandler handler)
MSA_MODULE_HOOK(void, unsubscribe, msa::Handle msa, Topic topic, EventHandler handler)
MSA_MODULE_HOOK(void, generate, msa::Handle msa, const Topic topic, void *args)
MSA_MODULE_HOOK(int16_t, schedule, msa::Handle msa, time_t timestamp, const Topic topic, void *args)
MSA_MODULE_HOOK(int16_t, delay, msa::Handle msa, std::chrono::milliseconds delay, const Topic topic, void *args)
MSA_MODULE_HOOK(int16_t, add_timer, msa::Handle msa, std::chrono::milliseconds period, const Topic topic, void *args)
MSA_MODULE_HOOK(void, remove_timer, msa::Handle msa, int16_t id)
MSA_MODULE_HOOK(void, get_timers, msa::Handle msa, std::vector<const Timer *> &list)

