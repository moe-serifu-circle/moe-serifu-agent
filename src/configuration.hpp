#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <wstring>
#include <string>

namespace msa { namespace config {

	typedef std::map<std::wstring, std::wstring> ConfigSection;

	typedef std::map<std::wstring, ConfigSection> Config;

	extern Config *load(const std::string &path);
	extern void save(const std::string &path, Config *configuration);

} }

#endif
