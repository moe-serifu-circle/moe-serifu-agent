#ifndef MSA_AGENT_HPP
#define MSA_AGENT_HPP

#include "msa.hpp"
#include "configuration.hpp"

#include <string>
#include <vector>

// Moe Serifu Agent state and manipulation

namespace msa { namespace agent {

	typedef enum state_type { idle, alert, listen, ero, converse, debug } State;
	typedef enum mood_type { normal } Mood;

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
	extern const Agent *get_agent(msa::Handle hdl);

	#define MSA_AGENT_PLUGIN_CALLABLE_FUNCS \
		X(void, say, const std::string &text) \
		X(void, register_substitution, const std::string &name) \
		X(void, set_substitution, const std::string &name, const std::string &value) \
		X(void, unregister_substitution, const std::string &name) \
		X(void, get_substitutions, std::vector<std::string> &subs)

	// declare hookables
	#define X(retspec, func, ...) extern retspec func(msa::Handle hdl, __VA_ARGS__);
	MSA_AGENT_PLUGIN_CALLABLE_FUNCS
	#undef X

	typedef struct plugin_hooks_type
	{
		#define X(retspec, func, ...) retspec (*func)(msa::Handle hdl, __VA_ARGS__);
		MSA_AGENT_PLUGIN_CALLABLE_FUNCS
		#undef X
	} PluginHooks;
	
} }
#endif
