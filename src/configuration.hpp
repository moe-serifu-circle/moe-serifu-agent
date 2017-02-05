#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>

namespace msa { namespace config {

	typedef std::map<std::string, std::string> ConfigSection;

	typedef std::map<std::string, ConfigSection> Config;

	extern Config *load(const std::string &path);
	extern void save(const std::string &path, Config *configuration);

} }

#endif
