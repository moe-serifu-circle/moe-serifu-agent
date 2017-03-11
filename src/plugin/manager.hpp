#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include "msa.hpp"
#include "plugin/plugin.hpp"

#include "configuration.hpp"

#include <string>

namespace msa { namespace plugin {

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern const std::string &load(msa::Handle hdl, const std::string &path);
	extern void unload(msa::Handle hdl, const std::string &id);
	extern void get_loaded(msa::Handle hdl, std::vector<std::string> &ids);
	extern void enable(msa::Handle hdl, const std::string &id);
	extern void disable(msa::Handle hdl, const std::string &id);
	extern bool is_enabled(msa::Handle hdl, std::string &id);
	
} }

