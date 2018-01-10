// Functions in the agent module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from agent module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include agent hooks before MSA_MODULE_HOOK macro is defined"
#endif

MSA_MODULE_HOOK(void, say, msa::Handle hdl, const std::string &text)
MSA_MODULE_HOOK(void, print_prompt_char, msa::Handle hdl)
MSA_MODULE_HOOK(void, register_substitution, msa::Handle hdl, const std::string &name)
MSA_MODULE_HOOK(void, set_substitution, msa::Handle hdl, const std::string &name, const std::string &value)
MSA_MODULE_HOOK(void, unregister_substitution, msa::Handle hdl, const std::string &name)
MSA_MODULE_HOOK(void, get_substitutions, msa::Handle hdl, std::vector<std::string> &subs)

