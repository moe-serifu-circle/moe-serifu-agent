#include "cmd.hpp"
#include "event/dispatch.hpp"
#include "string.hpp"
#include "agent.hpp"
#include "output.hpp"
#include "log.hpp"

#include <cstdio>
#include <stdexcept>
#include <map>

namespace msa { namespace cmd {

	struct command_type
	{
		CommandHandler handler;
		std::string *invoke;
		std::string *desc;
		std::string *usage;
	};

	struct command_context_type
	{
		bool running;
		static std::map<std::string, Command *> commands;
	};

	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static int create_command_context(CommandContext **ctx);
	static int dispose_command_context(CommandContext *ctx);

	// handlers
	static void announce_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void help_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void echo_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void exit_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);

	static void register_default_commands(msa::Handle hdl);
	static void unregister_default_commands(msa::Handle hdl);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		// need to init events before this
		if (hdl->event == NULL)
		{
			msa::log::error(hdl, "Tried to init CMD module before EVENT module started");
			return 1;
		}

		int status = create_command_context(&hdl->cmd);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not create event context (error " + std::to_string(status) + ")");
			return 1;
		}
		
		msa::event::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
		msa::event::subscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
		msa::event::subscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
		
		// check config to see if we should do an announce event
		try
		{
			read_config(hdl, config);
		}
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read cmd config: " + std::string(e.what()));
		}
		
		register_default_commands(hdl);
		
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		unregister_default_commands(hdl);
		int status = dispose_command_context(hdl->cmd);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not dispose command context (error " + std::to_string(status) + ")");
			return 1;
		}
		msa::event::unsubscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
		msa::event::unsubscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
		msa::event::unsubscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
		return 0;
	}

	extern void create_command(Command **cmd_ptr, const std::string &invoke, const std::string &desc, const std::string &usage, CommandHandler handler)
	{
		Command *cmd = new Command;
		cmd->invoke = new std::string(invoke);
		cmd->desc = new std::string(desc);
		cmd->usage = new std::string(usage);
		cmd->handler = handler;
		*cmd_ptr = cmd;
	}

	extern void dispose_handler(Command *cmd)
	{
		delete cmd->invoke;
		delete cmd->desc;
		delete cmd->usage;
		delete cmd;
	}

	extern void register_command(msa::Handle hdl, Command *cmd)
	{
		CommandContext *ctx = hdl->cmd;
		std::string invoke = std::string(*cmd->invoke);
		msa::string::to_upper(invoke);
		if (ctx->commands.find(invoke) != ctx->commands.end())
		{
			throw std::logic_error("command already exists: " + invoke);
		}
		ctx->commands[invoke] = cmd;
	}

	extern void unregister_command(msa::Handle hdl, Command *cmd)
	{
		CommandContext *ctx = hdl->cmd;
		std::string invoke = std::string(*cmd->invoke);
		msa::string::to_upper(invoke);
		if (ctx->commands.find(invoke) == ctx->commands.end())
		{
			throw std::logic_error("command does not exist: " + invoke);
		}
		ctx->commands.erase(invoke);
	}

	static void read_config(msa::Handle hdl, const msa::config::Section &config)
	{
		std::string do_anc = config.get_or("ANNOUNCE", "false");
		msa::string::to_upper(do_anc);
		if (do_anc == "TRUE" || do_anc == "YES" || do_anc == "1")
		{
			msa::event::generate(hdl, msa::event::Topic::COMMAND_ANNOUNCE, NULL);
		}
	}

	static int create_command_context(CommandContext **ctx)
	{
		command_context_type *c = new CommandContext;
		c->running = true;
		*ctx = c;
		return 0;
	}

	static int dispose_command_context(CommandContext *ctx)
	{
		delete ctx;
		return 0;
	}

	static void announce_func(msa::Handle hdl, const ArgList & UNUSED(args), msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		msa::output::write_text(hdl, a->name + ": \"I'd like to announce my presence!\"\n");
	}

	static void kill_func(msa::Handle hdl, const ArgList & UNUSED(args), msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		// copy the name because it can be overwritten on quit
		std::string name(a->name);
		msa::output::write_text(hdl, name + ": \"Right away master, I will terminate my EDT for you now!\"\n");
		int status = msa::quit(hdl);
		if (status != 0)
		{
			fprintf(stderr, "Shutdown error: %d\n", status);
		}
	}

	static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		std::string *str = static_cast<std::string *>(e->args);
		msa::output::write_text(hdl, a->name + ": \"I'm sorry, Master. I don't understand the command '" + *str + "'\"\n");
		delete str;
	}

	static void register_default_commands(msa::Handle hdl)
	{
		CommandContext *ctx = hdl->cmd;
		Command *kill_cmd, *announce_cmd, *echo_cmd, *help_cmd;
		create_command(&kill_cmd, "KILL", "Shuts down this MSA instance", "", kill_func);
		create_command(&announce_cmd, "ANNOUNCE", "Echoes a simple phrase to announce existance", "", announce_func);
		create_command(&echo_cmd, "ECHO", "Outputs the arguments", "echo-args...", echo_func);
		create_command(&help_cmd, "HELP", "Displays the help", "", help_func);
		register_command(hdl, kill_cmd);
		register_command(hdl, announce_cmd);
		register_command(hdl, echo_cmd);
		register_command(hdl, help_cmd);
	}

	static void unregister_default_commands(msa::Handle hdl)
	{
		Command *kill_cmd = hdl->cmd->commands["KILL"];
		Command *announce_cmd = hdl->cmd->commands["ANNOUNCE"];
		Command *echo_cmd = hdl->cmd->commands["ECHO"];
		Command *help_cmd = hdl->cmd->commands["HELP"];
		unregister_command(hdl, kill_cmd);
		unregister_command(hdl, announce_cmd);
		unregister_command(hdl, echo_cmd);
		unregister_command(hdl, help_cmd);
		dispose_command(kill_cmd);
		dispose_command(announce_cmd);
		dispose_command(echo_cmd);
		dispose_command(help_cmd);
	}

} }
