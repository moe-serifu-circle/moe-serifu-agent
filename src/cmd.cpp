#include "cmd.hpp"
#include "event/dispatch.hpp"
#include "string.hpp"
#include "agent.hpp"
#include "output.hpp"
#include "log.hpp"

#include <cstdio>
#include <exception>

namespace msa { namespace cmd {

	struct command_context_type
	{
		bool running;
	};

	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static int create_command_context(CommandContext **ctx);
	static int dispose_command_context(CommandContext *ctx);

	// handlers
	static void say_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
	static void exit_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
	static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

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
		
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
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

	static void say_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		msa::output::write_text(hdl, a->name + ": \"I'd like to announce my presence!\"\n");
	}

	static void exit_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
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

} }
