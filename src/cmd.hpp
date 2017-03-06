#ifndef CMD_HPP
#define CMD_HPP

// Handles definitions and specifications of commands

#include "msa.hpp"
#include "configuration.hpp"
#include "event/handler.hpp"

#include <vector>

namespace msa { namespace cmd {

	typedef struct command_type Command;
	typedef std::vector<std::string> ArgList;
	typedef void (*CommandHandler)(msa::Handle hdl, const ArgList &args, msa::event::HandlerSync *const sync);

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void create_command(Command **cmd, const std::string &invoke_name, const std::string &desc, const std::string usage, CommandHandler handler);
	extern void dispose_command(Command *cmd);
	extern void register_command(msa::Handle hdl, Command *cmd);
	extern void unregister_command(msa::Handle hdl, Command *cmd);

} }

#endif
