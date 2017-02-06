#include "agent.hpp"

#include <string>
#include <cstdint>

namespace msa { namespace agent {

	struct agent_context_type
	{
		Agent *agent;
	};

	Agent::agent_type(const std::string &n) : name(n), state(State::IDLE), attitude(0), mood(Mood::NORMAL)
	{}

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		AgentContext *ctx = new AgentContext;
		std::string name = std::string(config.get_or("NAME", "DEFAULT_NAME"));
		ctx->agent = new Agent(name);
		hdl->agent = ctx;
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		delete hdl->agent->agent;
		delete hdl->agent;
		return 0;
	}

	extern const Agent *get_agent(msa::Handle hdl)
	{
		return hdl->agent->agent;
	}

} }
