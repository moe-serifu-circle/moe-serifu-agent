#ifndef CMD_HPP
#define CMD_HPP

// Handles definitions and specifications of commands

#include "msa.hpp"
#include "configuration.hpp"

namespace msa { namespace cmd {

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);

} }

#endif
