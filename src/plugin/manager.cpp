#include "plugin/manager.hpp"

#include "log.hpp"

#include <map>
#include <exception>

#include "compat/file/file.hpp"

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
		std::string autoload_dir;
	};
	
	static int create_plugin_context(PluginContext **ctx_ptr);
	static int dispose_plugin_context(PluginContext *ctx);
	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static void load_all(msa::Handle hdl, const std::string &dir_path);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		if (create_plugin_context(&hdl->plugin) != 0)
		{
			msa::log::error(hdl, "Could not create plugin manager context");
			return -1;
		}
		try
		{
			read_config(hdl, config);
		}
		catch (std::exception &e)
		{
			msa::log::error(hdl, "Could not read config: " + e.what());
			return -1;
		}
		// do autoloading now
		if (hdl->plugin->autoload_dir != "")
		{
			load_all(hdl, hdl->plugin->autoload_dir);
		}
		return 0;
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
	
	extern const std::string &load(msa::Handle hdl, const std::string &path);
	extern void unload(msa::Handle hdl, const std::string &id);
	extern void get_loaded(msa::Handle hdl, std::vector<std::string> &ids);
	extern void enable(msa::Handle hdl, const std::string &id);
	extern void disable(msa::Handle hdl, const std::string &id);
	extern bool is_enabled(msa::Handle hdl, std::string &id);
	
	static int create_plugin_context(PluginContext **ctx_ptr)
	{
		PluginContext *ctx = new PluginContext;
		ctx->autoload_dir = "";
		*ctx_ptr = ctx;
		return 0;
	}
	
	static int dispose_plugin_context(PluginContext *ctx)
	{
		delete ctx;
		return 0;
	}

	static void read_config(msa::Handle hdl, const msa::config::Section &config)
	{
		if (!config.has("DIR"))
		{
			msa::log::warn(hdl, "No plugin directory specified in config; plugins will not be auto-loaded");
		}
		else
		{
			hdl->plugin->autoload_dir = config["DIR"];
		}
	}

	static void load_all(msa::Handle hdl, const std::string &dir_path)
	{
		std::vector<std::string> filenames;
		msa::file::list(dir_path, filenames);
		for (size_t i = 0; i < filesnames.size(); i++)
		{
			std::string fname = filenames[i];
			if (msa::string::ends_with(fname, ".so") || msa::string::ends_with(fname, ".dll"))
			{
				std::string full_path = dir_path;
				msa::file::join(full_path, fname);
				load(full_path);
			}
		}
	}

} }

