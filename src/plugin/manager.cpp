#include "plugin/manager.hpp"

#include "log.hpp"

#include <map>

namespace msa { namespace plugin {

	typedef struct plugin_entry_type
	{
		const Info *info;
		void *local_env;
		std::string *id;
	} PluginEntry;

	struct plugin_context_type
	{
		std::map<std::string, PluginEntry *> loaded;
		std::map<std::string, PluginEntry *> enabled;
	};
	
	static int create_plugin_context(PluginContext **ctx_ptr);
	static int dispose_plugin_context(PluginContext *ctx);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		if (create_plugin_context(&hdl->plugin) != 0)
		{
			msa::log::error(hdl, "Could not create plugin manager context");
			return -1;
		}
		
	}
	
	extern int quit(msa::Handle hdl)
	{
		if (dispose_plugin_context(hdl->plugin) != 0)
		{
			msa::log::error(hdl, "Clould not dispose plugin manager context");
			return -1;
		}
		return 0;
	}
	
	extern void load(msa::Handle hdl, const std::string &path, std::string &id);
	extern void unload(msa::Handle hdl, const std::string &id);
	extern void enable(msa::Handle hdl, const std::string &id);
	extern void disable(msa::Handle hdl, const std::string &id);
	
	static int create_plugin_context(PluginContext **ctx_ptr)
	{
		PluginContext *ctx = new PluginContext;
		*ctx_ptr = ctx;
		return 0;
	}
	
	static int dispose_plugin_context(PluginContext *ctx)
	{
		delete ctx;
		return 0;
	}

} }

