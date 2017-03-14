#include "plugin/plugin.hpp"

#include "log.hpp"
#include "cmd/cmd.hpp"
#include "agent.hpp"
#include "string.hpp"

#include "platform/file/file.hpp"
#include "platform/lib/lib.hpp"

#include <map>
#include <exception>

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
		std::vector<msa::cmd::Command *> commands;
	};
	
	static int create_plugin_context(PluginContext **ctx_ptr);
	static int dispose_plugin_context(PluginContext *ctx);
	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static void load_all(msa::Handle hdl, const std::string &dir_path);
	static bool call_plugin_add_commands(msa::Handle hdl, PluginEntry *entry);
	static bool call_plugin_func(msa::Handle hdl, const std::string &id, const std::string &func_name, Func func, void *local_env);
	static void cmd_enable(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const sync);
	static void cmd_disable(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const sync);
	static void cmd_list(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const sync);

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
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read config: " + std::string(e.what()));
			return -1;
		}
		// do autoloading now
		if (hdl->plugin->autoload_dir != "")
		{
			load_all(hdl, hdl->plugin->autoload_dir);
		}
		return 0;
	}
	
	extern int setup(msa::Handle hdl)
	{
		PluginContext *ctx = hdl->plugin;
		for (size_t i = 0; i < ctx->commands.size(); i++)
		{
			msa::cmd::register_command(hdl, ctx->commands[i]);
		}
		return 0;
	}
	
	extern int teardown(msa::Handle hdl)
	{
		PluginContext *ctx = hdl->plugin;
		for (size_t i = 0; i < ctx->commands.size(); i++)
		{
			msa::cmd::unregister_command(hdl, ctx->commands[i]);
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
			msa::log::error(hdl, "Loading library failed - could not find msa_plugin_getinfo symbol");
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
		if (is_loaded(hdl, *plugin_id))
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
		msa::log::info(hdl, "Unloading plugin with ID: " + id);
		PluginContext *ctx = hdl->plugin;
		if (!is_loaded(hdl, id))
		{
			msa::log::warn(hdl, "No plugin with ID; not unloading: " + id);
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
		msa::log::info(hdl, "Sucessfully unloaded plugin");
	}

	extern bool is_loaded(msa::Handle hdl, const std::string &id)
	{
		return (hdl->plugin->loaded.find(id) != hdl->plugin->loaded.end());
	}
	
	extern void get_loaded(msa::Handle hdl, std::vector<std::string> &ids)
	{
		PluginContext *ctx = hdl->plugin;
		std::map<std::string, PluginEntry *>::const_iterator iter;
		for (iter = ctx->loaded.begin(); iter != ctx->loaded.end(); iter++)
		{
			ids.push_back(iter->first);
		}
	}
	
	extern void enable(msa::Handle hdl, const std::string &id)
	{
		msa::log::info(hdl, "Enabling plugin '" + id + "'");
		PluginContext *ctx = hdl->plugin;
		if (!is_loaded(hdl, id))
		{
			throw std::logic_error("Plugin not loaded: " + id);
		}
		if (is_enabled(hdl, id))
		{
			throw std::logic_error("Plugin already enabled: " + id);
		}
		PluginEntry *entry = ctx->loaded[id];
		entry->local_env = NULL;
		if (entry->info->functions->init_func != NULL)
		{
			int status = 0;
			try
			{
				status = entry->info->functions->init_func(hdl, &entry->local_env);
			}
			catch (...)
			{
				msa::log::error(hdl, "Plugin '" + id + "' init_func threw an exception; plugin will be unloaded");
				unload(hdl, id);
				throw std::runtime_error("plugin unloaded; init() threw an exception");
			}
			if (status != 0)
			{
				msa::log::error(hdl, "Plugin '" + id + "': init function failed");
				msa::log::debug(hdl, id + "'s init_func return code is " + std::to_string(status));
				throw std::runtime_error("plugin unloaded; init() failed with code " + std::to_string(status));
			}
		}
		else
		{
			msa::log::warn(hdl, "Plugin '" + id + "' does not define an init_func; skipping calling init_func");
		}
		ctx->enabled[id] = entry;
		msa::log::info(hdl, "Loaded plugin with ID '" + id + "'");
		const FunctionTable *funcs = entry->info->functions;
		if (!call_plugin_func(hdl, id, "add_input_devices_func", funcs->add_input_devices_func, entry->local_env))
		{
			throw std::runtime_error("add_input_devices() failed");
		}
		if (!call_plugin_func(hdl, id, "add_output_devices_func", funcs->add_output_devices_func, entry->local_env))
		{
			throw std::runtime_error("add_output_devices() failed");
		}
		if (!call_plugin_func(hdl, id, "add_agent_props_func", funcs->add_agent_props_func, entry->local_env))
		{
			throw std::runtime_error("add_agent_props() failed");
		}
		if (!call_plugin_add_commands(hdl, entry))
		{
			throw std::runtime_error("add_commands() failed");
		}
	}
	
	extern void disable(msa::Handle hdl, const std::string &id)
	{
		msa::log::info(hdl, "Disabling plugin '" + id + "'...");
		PluginContext *ctx = hdl->plugin;
		if (!is_enabled(hdl, id))
		{
			return;
		}
		PluginEntry *entry = ctx->enabled[id];
		ctx->enabled.erase(id);
		if (entry->info->functions->quit_func != NULL)
		{
			int status = 0;
			try
			{
				status = entry->info->functions->quit_func(hdl, entry->local_env);
			}
			catch (...)
			{
				msa::log::error(hdl, "Plugin '" + id + "' quit_func threw an exception; plugin will be unloaded");
				unload(hdl, id);
				throw std::runtime_error("plugin unloaded; quit() threw an exception");
				
			}
			if (status != 0)
			{
				msa::log::error(hdl, "Plugin '" + id + "': quit function failed");
				msa::log::debug(hdl, id + "'s quit_func return code is " + std::to_string(status));
				unload(hdl, id);
				throw std::runtime_error("plugin unloaded; init() failed with code " + std::to_string(status));
			}
		}
		else
		{
			msa::log::info(hdl, "Plugin '" + id + "' does not define a quit_func; skipping calling quit_func");
		}
	}
	
	extern bool is_enabled(msa::Handle hdl, const std::string &id)
	{
		return (hdl->plugin->enabled.find(id) != hdl->plugin->enabled.end());
	}
	
	static void cmd_enable(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		if (args.size() < 1)
		{
			msa::agent::say(hdl, "Well sure, but you gotta tell me which plugin you want.");
			return;
		}
		std::string plugin_id = args[0];
		if (!is_loaded(hdl, plugin_id))
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but I never loaded a plugin called '" + plugin_id + "'.");
			return;
		}
		if (is_enabled(hdl, plugin_id))
		{
			msa::agent::say(hdl, "Ooh! I already enabled that plugin for you, $USER_TITLE.");
			return;
		}
		enable(hdl, plugin_id);
		msa::agent::say(hdl, "All right, $USER_TITLE! I've now enabled the plugin called '" + plugin_id + "'.");
	}
	
	static void cmd_disable(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		if (args.size() < 1)
		{
			msa::agent::say(hdl, "Well sure, but you gotta tell me which plugin you want.");
			return;
		}
		std::string plugin_id = args[0];
		if (!is_loaded(hdl, plugin_id))
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but I never loaded a plugin called '" + plugin_id + "'.");
			return;
		}
		if (!is_enabled(hdl, plugin_id))
		{
			msa::agent::say(hdl, "Ooh! That plugin isn't enabled, $USER_TITLE.");
			return;
		}
		disable(hdl, plugin_id);
		msa::agent::say(hdl, "All right, $USER_TITLE! I've now disabled the plugin called '" + plugin_id + "'.");
	}
	
	static void cmd_list(msa::Handle hdl, const msa::cmd::ArgList & UNUSED(args), msa::event::HandlerSync *const UNUSED(sync))
	{
		std::vector<std::string> ids;
		get_loaded(hdl, ids);
		if (ids.empty())
		{
			msa::agent::say(hdl, "Hmm, I actually haven't loaded any plugins at all.");
			return;
		}
		msa::agent::say(hdl, "Okay, $USER_TITLE. Here are the plugins that I've loaded:");
		for (size_t i = 0; i < ids.size(); i++)
		{
			std::string enable_string = is_enabled(hdl, ids[i]) ? "(enabled)" : "(disabled)";
			msa::agent::say(hdl, "'" + ids[i] + "' " + enable_string);
		}
		std::string plural = ids.size() > 1 ? "s" : "";
		msa::agent::say(hdl, "That's " + std::to_string(ids.size()) + " plugin" + plural + " in total.");
	}
	
	static bool call_plugin_func(msa::Handle hdl, const std::string &id, const std::string &func_name, Func func, void *local_env)
	{
		if (func != NULL)
		{
			int status = 0;
			try
			{
				status = func(hdl, local_env);
			}
			catch (...)
			{
				msa::log::error(hdl, "Plugin '" + id + "' " + func_name + " threw an exception; plugin will be unloaded");
				unload(hdl, id);
				throw std::runtime_error("plugin unloaded; " + func_name + "() threw an exception");
				return false;
			}
			if (status != 0)
			{
				msa::log::error(hdl, "Plugin '" + id + "': " + func_name + " failed");
				msa::log::debug(hdl, "Plugin '" + id + "': " + func_name + " return code is " + std::to_string(status));
				return false;
			}
		}
		else
		{
			msa::log::warn(hdl, "Plugin '" + id + "' does not define " + func_name + "; skipping execution");
		}
		return true;
	}
	
	static bool call_plugin_add_commands(msa::Handle hdl, PluginEntry *entry)
	{
		std::vector<msa::cmd::Command *> new_commands;
		if (entry->info->functions->add_commands_func != NULL)
		{
			int status = 0;
			try
			{
				status = entry->info->functions->add_commands_func(hdl, entry->local_env, new_commands);
			}
			catch (...)
			{
				msa::log::error(hdl, "Plugin '" + *entry->id + "' add_commands_func threw an exception; plugin will be unloaded");
				unload(hdl, *entry->id);
				throw std::runtime_error("plugin unloaded; add_commands() threw an exception");
				return false;
			}
			if (status != 0)
			{
				msa::log::error(hdl, "Plugin '" + *entry->id + "': add_commands_func failed");
				msa::log::debug(hdl, "Plugin '" + *entry->id + "': add_commands_func return code is " + std::to_string(status));
				return false;
			}
		}
		else
		{
			msa::log::info(hdl, "Plugin '" + *entry->id + "' does not define add_commands_func; skipping execution");
		}
		for (size_t i = 0; i < new_commands.size(); i++)
		{
			msa::cmd::register_command(hdl, new_commands[i]);
		}
		return true;
	}
	
	static int create_plugin_context(PluginContext **ctx_ptr)
	{
		PluginContext *ctx = new PluginContext;
		ctx->autoload_dir = "";
		ctx->commands.push_back(new msa::cmd::Command("PLUGINDISABLE", "It turns on a plugin", "plugin-id", cmd_enable));
		ctx->commands.push_back(new msa::cmd::Command("PLUGINENABLE", "It turns off a plugin", "plugin-id", cmd_disable));
		ctx->commands.push_back(new msa::cmd::Command("PLUGINLIST", "It lists all of the plugins", "", cmd_list));
		*ctx_ptr = ctx;
		return 0;
	}
	
	static int dispose_plugin_context(PluginContext *ctx)
	{
		for (size_t i = 0; i < ctx->commands.size(); i++)
		{
			delete ctx->commands[i];
		}
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
		for (size_t i = 0; i < filenames.size(); i++)
		{
			std::string fname = filenames[i];
			if (msa::string::ends_with(fname, ".so") || msa::string::ends_with(fname, ".dll"))
			{
				std::string full_path = dir_path;
				msa::file::join(full_path, fname);
				load(hdl, full_path);
			}
		}
	}

} }

