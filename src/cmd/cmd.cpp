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
		int last_status;
		bool last_threw_exception;
		std::map<std::string, const Command *> commands;
	};

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static int create_command_context(CommandContext **ctx);
	static int dispose_command_context(CommandContext *ctx);

	// handlers
	static void help_func(msa::Handle hdl, const ParamList &params, msa::event::HandlerSync *const sync);
	static void echo_func(msa::Handle hdl, const ParamList &params, msa::event::HandlerSync *const sync);
	static void kill_func(msa::Handle hdl, const ParamList &params, msa::event::HandlerSync *const sync);
	static void parse_command(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	static void register_default_commands(msa::Handle hdl);
	static void unregister_default_commands(msa::Handle hdl);
	
	static const Command default_commands[] = {
		{"KILL", "It shuts down this MSA instance", "", kill_func},
		{"ECHO", "It outputs its arguments", "echo-args...", echo_func},
		{"HELP", "With no args, it lists all commands. Otherwise, it displays the help", "[command]", help_func}
	};

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
	{
		// need to init events before this
		if (hdl->event == NULL)
		{
			msa::log::error(hdl, "Can't init CMD module before EVENT module started");
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
		catch (const msa::cfg::config_error &e)
		{
			msa::log::error(hdl, "Could not read cmd config: " + std::string(e.what()));
			return -1;
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
		std::string startup_cmd = config.get_or<std::string>("STARTUP", "echo I'd like to announce my presence!");
		msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, msa::event::wrap(startup_cmd));
	}

	static int create_command_context(CommandContext **ctx)
	{
		CommandContext *c = new CommandContext;
		c->last_status = 0;
		c->last_threw_exception = false;
		*ctx = c;
		return 0;
	}

	static int dispose_command_context(CommandContext *ctx)
	{
		delete ctx;
		return 0;
	}

	static void kill_func(msa::Handle hdl, const ParamList & UNUSED(params), msa::event::HandlerSync *const UNUSED(sync))
	{
		msa::agent::say(hdl, "Right away, $USER_TITLE, I will terminate my EDT for you now!");
		int status = msa::stop(hdl);
		if (status != 0)
		{
			fprintf(stderr, "Shutdown error: %d\n", status);
		}
	}
	
	static void help_func(msa::Handle hdl, const ParamList &params, msa::event::HandlerSync *const UNUSED(sync))
	{
		CommandContext *ctx = hdl->cmd;
		if (params.arg_count() > 0)
		{
			std::string cmd_name = params[0];
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
				
				// create the options:
				std::string opt_str;
				if (cmd->options != "")
				{
					bool has_non_arg_opts = false;
					opt_str = "";
					std::vector<char> arg_opts;
					for (size_t i = 0; i < cmd->options.size(); i++)
					{
						char ch = cmd->options[i];
						if (i + 1 < cmd->options.size() && cmd->options[i+1] == ':')
						{
							arg_opts.push_back(ch);
							i++;
						}
						else
						{
							if (!has_non_arg_opts)
							{
								opt_str += " [-";
								has_non_arg_opts = true;
							}
							opt_str += ch;
						}
					}
					if (has_non_arg_opts)
					{
						opt_str += "]";
					}
					for (auto iter = arg_opts.begin(); iter != arg_opts.end(); iter++)
					{
						opt_str += std::string(" [-") + *iter + " ARG]";
					}
				}
				msa::agent::say(hdl, "You can call it like this: " + cmd_name + opt_str + usage_str);
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
	
	static void echo_func(msa::Handle hdl, const ParamList &params, msa::event::HandlerSync *const UNUSED(sync))
	{
		std::string echo_string;
		for (size_t i = 0; i < params.arg_count(); i++)
		{
			echo_string += params[i];
			if (i + 1 < params.arg_count())
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
		std::vector<std::string> tokens;
		msa::string::tokenize(str, ' ', tokens);
		// pull out command name and call the appropriate function
		if (tokens.size() == 0)
		{
			// no command, terminate parsing
			return;
		}
		std::string cmd_name = tokens[0];
		msa::string::to_upper(cmd_name);
		if (ctx->commands.find(cmd_name) == ctx->commands.end())
		{
			msa::agent::say(hdl, "I'm sorry, $USER_TITLE. I don't know what you mean by '" + cmd_name + "'.");
		}
		else
		{
			const Command *cmd = ctx->commands[cmd_name];
			Result result(-1);
			try
			{
				ParamList params(tokens, cmd->options);
				result = cmd->handler(hdl, params, sync);
				ctx->last_threw_exception = false;
			}
			catch (const std::exception &e)
			{
				msa::log::error(hdl, "Command ''");
				msa::agent::say(hdl, "Oh no! I'm sorry, but I couldn't do that. Take a look at my log file.");
				ctx->last_threw_exception = true;
			}
			ctx->last_status = result.status();
		}
	}

	static void register_default_commands(msa::Handle hdl)
	{
		size_t num_commands = (sizeof(default_commands) / sizeof(Command));
		for (size_t i = 0; i < num_commands; i++)
		{
			const Command *cmd = &default_commands[i];
			register_command(hdl, cmd);
		}
	}

	static void unregister_default_commands(msa::Handle hdl)
	{
		size_t num_commands = (sizeof(default_commands) / sizeof(Command));
		for (size_t i = 0; i < num_commands; i++)
		{
			const Command *cmd = &default_commands[i];
			unregister_command(hdl, cmd);
		}
	}

	
	ParamList::ParamList(const std::vector<std::string> &tokens, const std::string &opts) :
		_command(tokens[0]),
		_args(),
		_options()
	{
		// start at 1 to skip the command name
		for (size_t tok_num = 1; tok_num < tokens.size(); tok_num++)
		{
			std::string tok = tokens[tok_num];
			if (tok[0] == '-' && tok.size() > 1 && tok[1] != '-')
			{
				// it's an option
				for (size_t i = 1; i < tok.size(); i++)
				{
					size_t pos;
					if (tok[i] != ':' && (pos = opts.find(tok[i])) != std::string::npos)
					{
						char cur_opt = tok[i];
						// do we need an option argument?
						if (opts.size() + 1 > pos && opts[pos + 1] == ':')
						{
							// check to make sure we are at the end of the opts string
							// and that there is at least one more non-opt arg
							if (i + 1 != tok.size() || tok_num + 1 == tokens.size())
							{
								throw std::runtime_error(std::string("option -") + cur_opt + " missing required argument");
							}
							// take opt arg, make sure it is not another opt
							tok_num++;
							std::string optarg = tokens[tok_num];
							if (optarg[0] == '-' && optarg.size() > 1 && optarg[1] != '-' && optarg[1] != ':')
							{
								throw std::runtime_error(std::string("option -") + cur_opt + " missing required argument");
							}
							_options[cur_opt].push_back(optarg);
						}
						else
						{
							_options[cur_opt].push_back("");
						}
					}
					else
					{
						throw std::runtime_error(std::string("unknown option -") + tok[i]);
					}
				}
			}
			else
			{
				_args.push_back(tok);
			}
		}
	}

	std::string ParamList::str() const
	{
		std::string str = command() + "(";
		for (size_t i = 0; i < arg_count(); i++)
		{
			str += std::string("\"") + get_arg(i) + "\"";
			if (i + 1 < arg_count())
			{
				str += ", ";
			}
		}
		if (!_options.empty())
		{
			str += " : "
			size_t num_opts = 0;
			for (auto i = _options.begin(); i != _options.end(); i++)
			{
				for (size_t j = 0; j < i->second.size(); j++)
				{
					str += "-" + i->first;
					const std::string &opt_arg = i->second[j];
					if (!i->second[j]->empty())
					{
						str += std::string("=\"") + *j + "\"";
					}
					if (j + 1 < i->second.size() || num_opts + 1 < _options.size())
					{
						str += " ";
					}
				}
				num_opts++;	
			}
			str += "]"
		}
		str += ")";
	}

	const std::string &ParamList::command() const
	{
		return _command;
	}

	const std::string &ParamList::operator[](size_t index) const
	{
		return get_arg(index);
	}

	const std::string &ParamList::get_arg(size_t index) const
	{
		return _args.at(index);
	}

	size_t ParamList::arg_count() const
	{
		return _args.size();
	}
	
	bool ParamList::has_option(char opt) const
	{
		return _options.find(opt) != _options.end();
	}
	
	const std::string &ParamList::get_option(char opt) const
	{
		return _options.at(opt).at(0);
	}
	
	size_t ParamList::option_count(char opt) const
	{
		return _options.at(opt).size();
	}
	
	const std::vector<std::string> &ParamList::all_option_args(char opt) const
	{
		return _options.at(opt);
	}

	Result::Result(int status) :
		_status(status),
		_value()
	{}

	Result::Result(int status, const std::string &retval) :
		_status(status),
		_value(retval)
	{}
	
	Result::Result(const std::string &retval) :
		_status(0),
		_value(retval)
	{}

	int Result::status() const
	{
		return _status;
	}

	const std::string &Result::value() const
	{
		return _value;
	}

} }
