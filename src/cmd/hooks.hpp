// Functions in the cmd module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from cmd module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include cmd hooks before MSA_MODULE_HOOK macro is defined"
#endif

MSA_MODULE_HOOK(void, get_commands, msa::Handle hdl, std::vector<const Command *> &list)

