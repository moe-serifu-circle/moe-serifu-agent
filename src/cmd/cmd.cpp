#include "cmd/cmd.hpp"
#include "event/dispatch.hpp"
#include "string.hpp"
#include "agent/agent.hpp"
#include "log.hpp"

#include <cstdio>
#include <stdexcept>
#include <map>

namespace msa { namespace cmd {

	struct command_context_type
	{
		bool running;
		std::map<std::string, const Command *> commands;
	};

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static int create_command_context(CommandContext **ctx);
	static int dispose_command_context(CommandContext *ctx);

	// handlers
	static void help_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void echo_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void kill_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void parse_command(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	static void register_default_commands(msa::Handle hdl);
	static void unregister_default_commands(msa::Handle hdl);
	
	static const struct command_type default_commands[] = {
		{"KILL", "It shuts down this MSA instance", "", kill_func},
		{"ECHO", "It outputs its arguments", "echo-args...", echo_func},
		{"HELP", "With no args, it lists all commands. Otherwise, it displays the help", "[command]", help_func}
	};

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
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
		
		register_default_commands(hdl);
		
		msa::event::subscribe(hdl, msa::event::Topic::TEXT_INPUT, parse_command);
		
		// check config to see if we should do an announce event
		try
		{
			read_config(hdl, config);
		}
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read cmd config: " + std::string(e.what()));
		}
		
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		msa::event::unsubscribe(hdl, msa::event::Topic::TEXT_INPUT, parse_command);
		unregister_default_commands(hdl);
		int status = dispose_command_context(hdl->cmd);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not dispose command context (error " + std::to_string(status) + ")");
			return 1;
		}
		return 0;
	}

	extern void dispose_handler(Command *cmd)
	{
		delete cmd;
	}

	extern void register_command(msa::Handle hdl, const Command *cmd)
	{
		CommandContext *ctx = hdl->cmd;
		std::string invoke = std::string(cmd->invoke);
		msa::string::to_upper(invoke);
		if (ctx->commands.find(invoke) != ctx->commands.end())
		{
			throw std::logic_error("command already exists: " + invoke);
		}
		ctx->commands[invoke] = cmd;
	}

	extern void unregister_command(msa::Handle hdl, const Command *cmd)
	{
		CommandContext *ctx = hdl->cmd;
		std::string invoke = std::string(cmd->invoke);
		msa::string::to_upper(invoke);
		if (ctx->commands.find(invoke) == ctx->commands.end())
		{
			throw std::logic_error("command does not exist: " + invoke);
		}
		ctx->commands.erase(invoke);
	}

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		std::string startup_cmd = config.get_or("STARTUP", "echo I'd like to announce my presence!");
		std::string *cmd_str = new std::string(startup_cmd);
		msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, cmd_str);
	}

	static int create_command_context(CommandContext **ctx)
	{
		CommandContext *c = new CommandContext;
		c->running = true;
		*ctx = c;
		return 0;
	}

	static int dispose_command_context(CommandContext *ctx)
	{
		delete ctx;
		return 0;
	}

	static void kill_func(msa::Handle hdl, const ArgList & UNUSED(args), msa::event::HandlerSync *const UNUSED(sync))
	{
		msa::agent::say(hdl, "Right away, $USER_TITLE, I will terminate my EDT for you now!");
		int status = msa::stop(hdl);
		if (status != 0)
		{
			fprintf(stderr, "Shutdown error: %d\n", status);
		}
	}
	
	static void help_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		CommandContext *ctx = hdl->cmd;
		if (args.size() > 0)
		{
			std::string cmd_name = args[0];
			msa::string::to_upper(cmd_name);
			if (ctx->commands.find(cmd_name) == ctx->commands.end())
			{
				msa::agent::say(hdl, "I'm sorry, $USER_TITLE, but I don't know about the command '" + cmd_name + "'.");
				msa::agent::say(hdl, "But if you do HELP with no args, I'll list the commands I do know!");
			}
			else
			{
				const Command *cmd = ctx->commands[cmd_name];
				msa::agent::say(hdl, "Oh yeah, that's the " + cmd_name + " command!");
				msa::agent::say(hdl, cmd->desc + ".");
				std::string usage_str = "";				
				if (cmd->usage != "")
				{
					usage_str = " " + cmd->usage;
				}
				msa::agent::say(hdl, "You can call it like this: " + cmd_name + usage_str);
			}
		}
		else
		{
			msa::agent::say(hdl, "Sure! I'll list the commands I know about.");
			std::map<std::string, const Command *>::const_iterator iter;
			for (iter = ctx->commands.begin(); iter != ctx->commands.end(); iter++)
			{
				msa::agent::say(hdl, iter->first);
			}
			msa::agent::say(hdl, "You can do HELP followed by the name of a command to find out more.");
		}
	}
	
	static void echo_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		std::vector<std::string>::const_iterator iter;
		std::string echo_string;
		for (iter = args.begin(); iter != args.end(); iter++)
		{
			echo_string += *iter;
			if (iter + 1 != args.end())
			{
				echo_string += " ";
			}
		}
		msa::agent::say(hdl, echo_string);
	}
	
	static void parse_command(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync)
	{
		CommandContext *ctx = hdl->cmd;
		std::string *str = static_cast<std::string *>(e->args);
		std::vector<std::string> args;
		msa::string::tokenize(*str, ' ', args);
		delete str;
		// pull out command name and call the appropriate function
		if (args.size() == 0)
		{
			// no command, terminate parsing
			return;
		}
		std::string cmd_name = args[0];
		args.erase(args.begin());
		msa::string::to_upper(cmd_name);
		if (ctx->commands.find(cmd_name) == ctx->commands.end())
		{
			msa::agent::say(hdl, "I'm sorry, $USER_TITLE. I don't know what you mean by '" + cmd_name + "'.");
		}
		else
		{
			try
			{
				ctx->commands[cmd_name]->handler(hdl, args, sync);
			}
			catch (const std::exception &e)
			{
				msa::agent::say(hdl, "Oh no! I'm sorry, but I failed: " + std::string(e.what()));
			}
		}
	}

	static void register_default_commands(msa::Handle hdl)
	{
		size_t num_commands = (sizeof(default_commands) / sizeof(struct command_type));
		for (size_t i = 0; i < num_commands; i++)
		{
			const Command *cmd = &default_commands[i];
			register_command(hdl, cmd);
		}
	}

	static void unregister_default_commands(msa::Handle hdl)
	{
		size_t num_commands = (sizeof(default_commands) / sizeof(struct command_type));
		for (size_t i = 0; i < num_commands; i++)
		{
			const Command *cmd = &default_commands[i];
			unregister_command(hdl, cmd);
		}
	}

} }
