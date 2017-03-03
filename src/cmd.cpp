#include "cmd.hpp"
#include "event/dispatch.hpp"
#include "string.hpp"
#include "agent.hpp"
#include "output.hpp"

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
		msa::string::to_upper(do_anc);
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
		char buf[100];
		sprintf(buf, "%s: \"I'd like to announce my presence!\"\n", a->name.c_str());
		msa::output::write_text(hdl, std::string(buf));
	}

	static void exit_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		// copy the name because it can be overwritten on quit
		std::string name(a->name);
		char buf[100];
		sprintf(buf, "%s: \"Right away master, I will terminate my EDT for you now!\"\n", name.c_str());
		msa::output::write_text(hdl, std::string(buf));
		sprintf(buf, "%s: \"I'd like to announce my presence!\"\n", a->name.c_str());
		msa::output::write_text(hdl, std::string(buf));
		int status = msa::quit(hdl);
		sprintf(buf, "%s: \"Environment Status: %d\"\n", name.c_str(), hdl->status);
		msa::output::write_text(hdl, std::string(buf));
		if (status == 0)
		{
			sprintf(buf, "%s: \"System shutdown.\"\n", name.c_str());
		}
		else
		{
			sprintf(buf, "%s: \"Warning! could not quit: %d\"\n", name.c_str(), status);
		}
		msa::output::write_text(hdl, std::string(buf));
	}

	static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
	{
		const msa::agent::Agent *a = msa::agent::get_agent(hdl);
		std::string *str = static_cast<std::string *>(e->args);
		char buf[100];
		sprintf(buf, "%s: \"I'm sorry, Master. I don't understand the command '%s'\"\n", a->name.c_str(), str->c_str());
		delete str;
		msa::output::write_text(hdl, std::string(buf));
	}

} }
