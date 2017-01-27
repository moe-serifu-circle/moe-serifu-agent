#include "agent.hpp"

namespace agent {

	struct agent_t
	{
		// current activity
		MODE mode;
		
		// positive attitude to the master user,
		// TODO: make this into an ID->attitude table
		uint_32 attitude;
	
		// current emotional state, affected by context and responses
		MOOD mood;
	};

}
