#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>

namespace msa { namespace config {

	class Section
	{
		public:
			Section(const char *name);
			Section(const std::string &name);
			bool has(const char *key) const;
			bool has(const std::string &key) const;
			const char *get_or(const char *key, const char *def) const;
			const std::string &get_or(const std::string &key, const std::string &def) const;
			const std::string &operator[](const char *key) const;
			const std::string &operator[](const std::string &key) const;
			Section &operator=(const Section &sec);
			std::string &operator[](const char *key);
			std::string &operator[](const std::string &key);
			const std::string &get_name() const;
			const std::map<std::string, std::string> &get_entries() const;
		
		private:
			std::string name;
			std::map<std::string, std::string> entries;
	};

	typedef std::map<std::string, Section> Config;

	extern Config *load(const std::string &path);
	extern void save(const std::string &path, Config *configuration);

} }

#endif
