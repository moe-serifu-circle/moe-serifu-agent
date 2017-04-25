#ifndef MSA_INPUT_INPUT_HPP
#define MSA_INPUT_INPUT_HPP

#include "msa.hpp"
#include "cfg/cfg.hpp"

#include <vector>
#include <string>

namespace msa { namespace input {

	typedef enum input_type_type { TTY, TCP, UDP } InputType;

	typedef struct chunk_type
	{
		std::string text;
	} Chunk;

	typedef struct device_type Device;

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void add_device(msa::Handle hdl, InputType type, void *device_id);
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "input/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "input/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
} }

#endif
