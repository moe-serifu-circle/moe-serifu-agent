#ifndef PLUGIN_HPP
#define PLUGIN_HPP

// will look for a function called 'msa_plugin_getinfo()' and 'msa_plugin_create_env()'
// their types must be PluginGetInfoFunc and PluginCreateEnvFunc respectively.

#include "msa.hpp"

#include <cstdint>

namespace msa { namespace plugin

	typedef const msa::plugin::Info *(*PluginGetInfoFunc)();
	typedef void *(*PluginCreateEnvFunc);
	typedef int (*PluginFunc)(msa::Handle hdl, void *plugin_env);

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
		PluginFunc init_func;
		PluginFunc quit_func;
		PluginFunc add_input_devices_func;
		PluginFunc add_output_devices_func;
		PluginFunc add_agent_props_func;
		PluginFunc add_commands_func;
	} Info;

	static inline void init_info(PluginInfo *info)
	{
		info->name = "";
		info->authors = NULL;
		info->num_authors = 0;
		info->version->major = 0;
		info->version->minor = 0;
		info->version->debug = 0;
		info->version->build = 0;
		info->init_func = NULL;
		info->quit_func = NULL;
		info->add_input_devices_func = NULL;
		info->add_output_devices_func = NULL;
		info->add_agent_props_func = NULL;
		info->add_commands_func = NULL;
	}

	extern int init(msa::Handle hdl);
	extern int quit(msa::Handle hdl);
	extern void load(msa::Handle hdl, const char *path);
	extern void unload(msa::Handle hdl, const char *id);

} }

#endif
