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
MSA_MODULE_HOOK(void, generate, msa::Handle msa, const Topic topic, const IArgs &args)
