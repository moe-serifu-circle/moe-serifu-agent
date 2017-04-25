// Functions in the log module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from log module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include log hooks before MSA_MODULE_HOOK macro is defined"
#endif


MSA_MODULE_HOOK(void, trace, msa::Handle hdl, const std::string &msg)
MSA_MODULE_HOOK(void, debug, msa::Handle hdl, const std::string &msg)
MSA_MODULE_HOOK(void, info, msa::Handle hdl, const std::string &msg)
MSA_MODULE_HOOK(void, warn, msa::Handle hdl, const std::string &msg)
MSA_MODULE_HOOK(void, error, msa::Handle hdl, const std::string &msg)
MSA_MODULE_HOOK(Level, get_level, msa::Handle hdl)

