// Functions in the output module that dynamically-loaded modules (plugins) are allowed to call.

// Define the MSA_MODULE_HOOK macro to arrange the hooks as needed, and then include this file.
// MSA_MODULE_HOOK will declare hooks with the following arguments:
// MSA_MODULE_HOOK(return-spec, func-name, ...) where ... is the arguments of the hook
// This file should only be included from output module code.

#ifndef MSA_MODULE_HOOK
	#error "cannot include output hooks before MSA_MODULE_HOOK macro is defined"
#endif

MSA_MODULE_HOOK(void, write, msa::Handle hdl, const Chunk *chunk)
MSA_MODULE_HOOK(void, write_text, msa::Handle hdl, const std::string &text)
MSA_MODULE_HOOK(void, switch_device, msa::Handle hdl, const std::string &id)
MSA_MODULE_HOOK(void, get_devices, msa::Handle hdl, std::vector<std::string> *list)
MSA_MODULE_HOOK(void, get_active_device, msa::Handle hdl, std::string &id)
MSA_MODULE_HOOK(void, create_chunk, Chunk **chunk, const std::string &text)
MSA_MODULE_HOOK(void, dispose_chunk, Chunk *chunk)
MSA_MODULE_HOOK(void, create_handler, OutputHandler **handler, const std::string &name, OutputHandlerFunc func)
MSA_MODULE_HOOK(void, dispose_handler, OutputHandler *handler)
MSA_MODULE_HOOK(const std::string&, get_handler_name, const OutputHandler *handler)

