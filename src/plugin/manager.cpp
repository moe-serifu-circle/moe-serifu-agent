#include "plugin/manager.hpp"

#include "log.hpp"

#include <map>
#include <exception>

#include "compat/file/file.hpp"
#include "compat/lib/lib.hpp"

namespace msa { namespace plugin {

	static const std::string BAD_PLUGIN_ID = "";

	typedef struct plugin_entry_type
	{
		const Info *info;
		void *local_env;
		std::string *id;
		msa::lib::Library *lib;
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
	
	extern const std::string &load(msa::Handle hdl, const std::string &path)
	{
		msa::log::info(hdl, "Loading plugin library " + path);
		PluginContext *ctx = hdl->plugin;
		msa::lib::Library *lib = msa::lib::open(path);
		GetInfoFunc get_info = NULL;
		// check that plugin has a getinfo()
		try
		{
			get_info = msa::lib::get_symbol<GetInfoFunc>(lib, "msa_plugin_getinfo");
		}
		catch (const msa::lib::library_error &e)
		{
			msa::lib::close(lib);
			msa::log::error(hdl, "Loading library failed");
			return BAD_PLUGIN_ID;
		}
		const msa::plugin::Info *info = NULL;
		// check if plugin's getinfo() throws
		try
		{
			info = get_info();
		}
		catch (...)
		{
			msa::log::error(hdl, "Plugin's msa_plugin_getinfo() function threw an error");
			msa::lib::close(lib);
			return BAD_PLUGIN_ID;
		}
		// check that plugin's getinfo() returns a real pointer
		if (info == NULL)
		{
			msa::log::error(hdl, "Plugin's msa_plugin_getinfo() function returned NULL");
			msa::lib::close(lib);
			return BAD_PLUGIN_ID;
		}
		std::string *plugin_id = new std::string(info->name);
		// check that we have not already loaded this plugin
		if (ctx->loaded.find(*plugin_id) != ctx->loaded.end())
		{
			msa::log::warn(hdl, "Plugin ID is already loaded: " + *plugin_id);
			msa::lib::close(lib);
			delete plugin_id;
			return BAD_PLUGIN_ID;
		}
		// okay, we finally have a valid info table extracted from plugin. now add an entry
		PluginEntry *entry = new PluginEntry;
		entry->info = info;
		entry->local_env = NULL;
		entry->id = plugin_id;
		entry->lib = lib;
		ctx->loaded[*plugin_id] = entry;
		msa::log::info(hdl, "Loaded plugin with ID: " + *plugin_id);
		return *plugin_id;
	}
	
	extern void unload(msa::Handle hdl, const std::string &id)
	{
		msa::log::info("Unloading plugin with ID: " + id);
		PluginContext *ctx = hdl->plugin;
		if (ctx->loaded.find(id) == ctx->loaded.end())
		{
			msa::log::warn("No plugin with ID; not unloading: " + id);
			return;
		}
		if (is_enabled(hdl, id))
		{
			disable(hdl, id);
		}
		PluginEntry *entry = ctx->loaded[id];
		try
		{
			msa::lib::close(entry->lib);
		}
		catch (const msa::lib::library_error &e)
		{
			msa::log::error(hdl, "Could not unload plugin library " + e.name());
			return;
		}
		ctx->loaded.erase(id);
		delete entry->id;
		delete entry;
		msa::log::info("Sucessfully unloaded plugin");
	}
	
	extern void get_loaded(msa::Handle hdl, std::vector<std::string> &ids)
	{
		
	}
	
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

