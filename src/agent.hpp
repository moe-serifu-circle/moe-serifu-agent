#ifndef AGENT_H
#define AGENT_H

// Moe Serifu Agent state and manipulation

enum msa_mode { IDLE, ALERT, LISTEN, ERO, CONVERSE, DEBUG };

enum msa_mood { NEUTRAL };

typedef struct
{
	// current activity
	msa_state mode;
	
	// positive attitude to the master user
	int attitude;

	// current emotional state, affected by context and responses
	msa_mood mood;
} agent;

#endif
