#include "cmd.hpp"
#include "event/dispatch.hpp"
#include "string.hpp"
#include "agent.hpp"

#include <cstdio>

namespace msa { namespace cmd {

	struct command_context_type
	{
		bool running;
	};

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
			return 1;
		}
		if (create_command_context(&hdl->cmd) != 0)
		{
			return 1;
		}
		msa::event::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
		msa::event::subscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
		msa::event::subscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
		
		// check config to see if we should do an announce event		
		std::string do_anc = config.get_or("ANNOUNCE", "false");
		msa::util::to_upper(do_anc);
		if (do_anc == "TRUE" || do_anc == "YES" || do_anc == "1")
		{	
			msa::event::generate(hdl, msa::event::Topic::COMMAND_ANNOUNCE, NULL);
		}
		
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		if (dispose_command_context(hdl->cmd) != 0)
		{
			return 1;
		}
		msa::event::unsubscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
		msa::event::unsubscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
		msa::event::unsubscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
		return 0;
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
		printf("%s: \"I'd like to announce my presence!\"\n", a->name.c_str());
	}

	static void exit_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		const char *name = a->name.c_str();
		printf("%s: \"Right away master, I will terminate my EDT for you now!\"\n", name);
		int status = msa::quit(hdl);
		printf("%s: \"Environment Status: %d\"\n", name, hdl->status);
		if (status == 0)
		{
			printf("%s: \"System shutdown.\"\n", name);
		}
		else
		{
			printf("%s: \"Warning! could not quit: %d\"\n", name, status);
		}
	}

	static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		std::string *str = static_cast<std::string *>(e->args);
		printf("%s: \"I'm sorry, Master. I don't understand the command '%s'\"\n", a->name.c_str(), str->c_str());
		delete str;
	}

} }
