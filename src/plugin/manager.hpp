#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include "msa.hpp"
#include "plugin/plugin.hpp"

#include "configuration.hpp"

#include <string>

namespace msa { namespace plugin {

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void load(msa::Handle hdl, const std::string &path, std::string &id);
	extern void unload(msa::Handle hdl, const std::string &id);
	extern void enable(msa::Handle hdl, const std::string &id);
	extern void disable(msa::Handle hdl, const std::string &id);
	
} }

