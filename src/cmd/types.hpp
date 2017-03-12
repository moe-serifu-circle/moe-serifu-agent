#ifndef CMD_TYPES_HPP
#define CMD_TYEPS_HPP

#include "event/handler.hpp"

#include <vector>
#include <string>

namespace msa { namespace cmd {
	
	typedef std::vector<std::string> ArgList;
	typedef void (*CommandHandler)(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);
	
	typedef struct command_type
	{
		command_type(const std::string &invoke, const std::string &desc, const std::string &usage, CommandHandler handler) :
			invoke(invoke), desc(desc), usage(usage), handler(handler)
		{}
		
		std::string invoke;
		std::string desc;
		std::string usage;
		CommandHandler handler;
	} Command;

} }

#endif
