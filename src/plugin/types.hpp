#ifndef PLUGIN_TYPES_HPP
#define PLUGIN_TYPES_HPP

// will look for a function called 'msa_plugin_getinfo()' which must be a GetInfoFunc.

#include "msa.hpp"
#include "cmd/types.hpp"

#include <cstdint>
#include <vector>

namespace msa { namespace plugin {

	typedef struct info_type Info;

	typedef const Info *(*GetInfoFunc)(void);
	typedef int (*Func)(msa::Handle hdl, void *plugin_env);
	typedef int (*AddCommandsFunc)(msa::Handle hdl, void *plugin_env, std::vector<msa::cmd::Command *> &new_commands);
	typedef int (*InitFunc)(msa::Handle hdl, void **plugin_env);

	typedef struct version_type
	{
		uint32_t major;
		uint32_t minor;
		uint32_t debug;
		uint32_t build;
	} Version;

	struct info_type
	{
		const char *name;
		std::vector<std::string> authors;
		Version version;
		InitFunc init_func;
		Func quit_func;
		Func add_input_devices_func;
		Func add_output_devices_func;
		Func add_agent_props_func;
		AddCommandsFunc add_commands_func;
		info_type() :
			name(""),
			authors(),
			version({0, 0, 0, 0}),
			init_func(NULL),
			quit_func(NULL),
			add_input_devices_func(NULL),
			add_output_devices_func(NULL),
			add_agent_props_func(NULL),
			add_commands_func(NULL)
			{}
	};

} }

#endif
