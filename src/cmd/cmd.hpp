#ifndef CMD_CMD_HPP
#define CMD_CMD_HPP

// Handles definitions and specifications of commands

#include "msa.hpp"
#include "configuration.hpp"
#include "cmd/types.hpp"

namespace msa { namespace cmd {
	
	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void register_command(msa::Handle hdl, const Command *cmd);
	extern void unregister_command(msa::Handle hdl, const Command *cmd);
} }

#endif
