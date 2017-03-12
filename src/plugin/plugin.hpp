#ifndef PLUGIN_PLUGIN_HPP
#define PLUGIN_PLUGIN_HPP

// will look for a function called 'msa_plugin_getinfo()' which must be a GetInfoFunc.

#include "msa.hpp"

#include <cstdint>

namespace msa { namespace plugin

	typedef const Info *(*GetInfoFunc)(void);
	typedef int (*Func)(msa::Handle hdl, void *plugin_env);
	typedef int (*InitFunc)(msa::Handle hdl, void **plugin_env);

	typedef struct info_type
	{
		const char *name;
		const char *authors[];
		size_t num_authors;
		struct
		{
			uint32_t major;
			uint32_t minor;
			uint32_t debug;
			uint32_t build;
		} version;
		InitFunc init_func;
		Func quit_func;
		Func add_input_devices_func;
		Func add_output_devices_func;
		Func add_agent_props_func;
		Func add_commands_func;
		info_type() :
			name(""),
			authors(NULL),
			num_authors(0),
			version({0, 0, 0, 0}),
			init_func(NULL),
			quit_func(NULL),
			add_input_devices_func(NULL),
			add_output_devices_func(NULL),
			add_agent_props_func(NULL),
			add_commands_func(NULL)
			{}
	} Info;

} }

#endif
