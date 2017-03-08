#ifndef AGENT_HPP
#define AGENT_HPP

#include "msa.hpp"
#include "configuration.hpp"

#include <string>
#include <vector>

// Moe Serifu Agent state and manipulation

namespace msa { namespace agent {

	typedef enum state_type { IDLE, ALERT, LISTEN, ERO, CONVERSE, DEBUG } State;
	typedef enum mood_type { NORMAL } Mood;

	typedef struct agent_type
	{
		// name of the agent
		std::string name;

		// current activity
		State state;
		
		// positive attitude to the master user,
		// TODO: make this into an ID->attitude table
		uint32_t attitude;
			
		// current emotional state, affected by context and responses
		Mood mood;

		// creates a new agent, n is the name of the agent
		agent_type(const std::string &n);
	} Agent;

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void say(msa::Handle hdl, const std::string &text);
	extern void register_substitution(msa::Handle hdl, const std::string &name);
	extern void set_substitution(msa::Handle hdl, const std::string &name, const std::string &value);
	extern void unregister_substitution(msa::Handle hdl, const std::string &name);
	extern void get_substitutions(msa::Handle hdl, std::vector<std::string> &subs);
	extern const Agent *get_agent(msa::Handle hdl);
	
} }
#endif
