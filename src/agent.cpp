#include "agent.hpp"

#include <string>
#include <cstdint>

namespace msa { namespace agent {

	struct agent_context_type
	{
		Agent *agent;
	};

	Agent::agent_type(const std::string &n) : name(n), attitude(0), mood(Mood::NORMAL), state(State::IDLE)
	{}

	extern int init(msa::Handle *hdl)
	{
		AgentContext *ctx = new AgentContext;
		ctx->agent = new Agent("Masa-chan");
		hdl->agent = ctx;
		return 0;
	}

	extern int quit(msa::Handle *hdl)
	{
		delete hdl->agent->agent;
		delete hdl->agent;
	}

} }
