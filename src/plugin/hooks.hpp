// Functions in the plugin module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from plugin module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include plugin hooks before MSA_MODULE_HOOK macro is defined"
#endif

MSA_MODULE_HOOK(void, get_loaded, msa::Handle hdl, std::vector<std::string> &ids)
MSA_MODULE_HOOK(bool, is_enabled, msa::Handle hdl, const std::string &id)
MSA_MODULE_HOOK(bool, is_loaded, msa::Handle hdl, const std::string &id)
MSA_MODULE_HOOK(const Info*, get_info, msa::Handle hdl, const std::string &id)

