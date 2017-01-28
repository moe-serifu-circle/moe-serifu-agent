#ifndef AGENT_HPP
#define AGENT_HPP

// Moe Serifu Agent state and manipulation

namespace msa { namespace agent {

	typedef enum mode_type { IDLE, ALERT, LISTEN, ERO, CONVERSE, DEBUG } Mode;
	typedef enum mood_type { NEUTRAL } Mood;

	typedef struct agent_type Agent;
	
} }
#endif
