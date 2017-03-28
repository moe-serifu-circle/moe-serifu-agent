#ifndef MSA_AGENT_HPP
#define MSA_AGENT_HPP

#include "msa.hpp"
#include "configuration.hpp"

// Start of hooks' includes
#include <string>
#include <vector>
// End of hooks' includes

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
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "agent_hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "agent_hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
	
} }
#endif
