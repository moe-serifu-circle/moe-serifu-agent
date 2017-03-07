#include "agent.hpp"
#include "log.hpp"
#include "output.hpp"

#include <string>
#include <cstdint>
#include <exception>

namespace msa { namespace agent {

	struct agent_context_type
	{
		Agent *agent;
	};

	Agent::agent_type(const std::string &n) : name(n), state(State::IDLE), attitude(0), mood(Mood::NORMAL)
	{}

	static int create_agent_context(AgentContext **ctx);
	static int dispose_agent_context(AgentContext *ctx);
	static void read_config(msa::Handle hdl, const msa::config::Section &config);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		int status = create_agent_context(&hdl->agent);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not create agent context");
		}
		try
		{
			read_config(hdl, config);
		}
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read agent config: " + std::string(e.what()));
		}
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		int status = dispose_agent_context(hdl->agent);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not dispose agent context");
		}
		return 0;
	}

	extern const Agent *get_agent(msa::Handle hdl)
	{
		return hdl->agent->agent;
	}

	extern void say(msa::Handle hdl, const std::string &text)
	{
		Agent *a = hdl->agent->agent;
		msa::output::write_text(hdl, a->name + ": \"" + text + "\"\n");
	}

	static int create_agent_context(AgentContext **ctx_ptr)
	{
		AgentContext *ctx = new AgentContext;
		ctx->agent = NULL;
		*ctx_ptr = ctx;
		return 0;
	}

	static int dispose_agent_context(AgentContext *ctx)
	{
		if (ctx->agent != NULL)
		{
			delete ctx->agent;
		}
		delete ctx;
		return 0;
	}

	static void read_config(msa::Handle hdl, const msa::config::Section &config)
	{
		std::string name = std::string(config.get_or("NAME", "DEFAULT_NAME"));
		hdl->agent->agent = new Agent(name);
	}

} }
