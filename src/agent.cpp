#include "agent.hpp"

#include <cstdint>

namespace msa { namespace agent {

	struct agent_type
	{
		// current activity
		Mode mode;
		
		// positive attitude to the master user,
		// TODO: make this into an ID->attitude table
		uint32_t attitude;
	
		// current emotional state, affected by context and responses
		Mood mood;
	};

} }
