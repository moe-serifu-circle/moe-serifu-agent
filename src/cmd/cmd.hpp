#ifndef CMD_CMD_HPP
#define CMD_CMD_HPP

// Handles definitions and specifications of commands

#include "msa.hpp"
#include "configuration.hpp"
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
	
	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void register_command(msa::Handle hdl, const Command *cmd);
	extern void unregister_command(msa::Handle hdl, const Command *cmd);
} }

#endif
