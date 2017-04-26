#include "cmd/cmd.hpp"
#include "event/dispatch.hpp"
#include "util/string.hpp"
#include "agent/agent.hpp"
#include "log/log.hpp"

#include <cstdio>
#include <stdexcept>
#include <map>
#include <chrono>

namespace msa { namespace cmd {

	static const PluginHooks HOOKS = {
		#define MSA_MODULE_HOOK(retspec, name, ...)		name,
		#include "cmd/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};

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
	static void timer_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void deltimer_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	static void parse_command(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	static void register_default_commands(msa::Handle hdl);
	static void unregister_default_commands(msa::Handle hdl);
	
	static const struct command_type default_commands[] = {
		{"KILL", "It shuts down this MSA instance", "", kill_func},
		{"ECHO", "It outputs its arguments", "echo-args...", echo_func},
		{"TIMER", "It schedules a command to execute in the future", "[-r] time-ms command", timer_func},
		{"DELTIMER", "It deletes a timer", "timer-id", deltimer_func},
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

	extern const PluginHooks *get_plugin_hooks()
	{
		return &HOOKS;
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
	
	extern void get_commands(msa::Handle hdl, std::vector<const Command *> &list)
	{
		CommandContext *ctx = hdl->cmd;
		std::map<std::string, const Command *>::const_iterator iter;
		for (iter = ctx->commands.begin(); iter != ctx->commands.end(); iter++)
		{
			list.push_back(iter->second);
		}
	}

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		std::string startup_cmd = config.get_or("STARTUP", "echo I'd like to announce my presence!");
		msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, msa::event::wrap(startup_cmd));
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

	static void timer_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		bool recurring = false;
		size_t cur_arg = 0;
		recurring = args.size() >= 1 && args[0] == "-r";
		if (recurring) cur_arg++;
		if (args.size() < cur_arg + 2)
		{
			msa::agent::say(hdl, "You gotta give me a time and a command to execute, $USER_TITLE.");
			return;
		}
		int delay = 0;
		try
		{
			delay = std::stoi(args[cur_arg]);
		}
		catch (std::exception &e)
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but '" + args[cur_arg] + "' isn't a number of milliseconds.");
			return;
		}
		cur_arg++;
		auto ms = std::chrono::milliseconds(delay);
		std::string cmd_str = "";
		for (size_t i = cur_arg; i < args.size(); i++)
		{
			cmd_str += args[i];
			if (i + 1 < args.size())
			{
				cmd_str += " ";
			}
		}
		std::string plural = ms.count() != 0 ? "s" : "";
		std::string type = recurring ? "every" : "in";
		int16_t id = -1;
		if (recurring)
		{
			id = msa::event::add_timer(hdl, ms, msa::event::Topic::TEXT_INPUT, msa::event::wrap(cmd_str));
		}
		else
		{
			id = msa::event::delay(hdl, ms, msa::event::Topic::TEXT_INPUT, msa::event::wrap(cmd_str));
		}
		if (id == -1)
		{
			msa::agent::say(hdl, "Oh no! I'm sorry, $USER_TITLE, that didn't work quite right!");
		}
		else
		{
			msa::agent::say(hdl, "Okay, $USER_TITLE, I will do that " + type + " " + std::to_string(ms.count()) + " millisecond" + plural + "!");
			msa::agent::say(hdl, "The timer ID is " + std::to_string(id) + ".");
		}
	}	

	static void deltimer_func(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const UNUSED(sync))
	{
		if (args.size() < 1)
		{
			msa::agent::say(hdl, "Ahh... $USER_TITLE, I need to know which timer I should delete.");
			return;
		}
		int16_t id = 0;
		try
		{
			id = std::stoi(args[0]);
		}
		catch (std::exception &e)
		{
			msa::agent::say(hdl, "Sorry, $USER_TITLE, but '" + args[0] + "' isn't an integer.");
			return;
		}
		msa::event::remove_timer(hdl, id);
		msa::agent::say(hdl, "Okay! I stopped timer " + std::to_string(id) + " for you, $USER_TITLE.");
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
		auto e_args = dynamic_cast<msa::event::Args<std::string>*>(e->args);
		std::string str = e_args->get_args();
		delete e_args;
		std::vector<std::string> args;
		msa::string::tokenize(str, ' ', args);
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
