#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>
#include <vector>

namespace msa { namespace config {

	/**
	 * Use the config section like a collection of values (with get_all()), or you can just
	 * use it as though there is only one value per key (use the [] operator to get the
	 * first value in the collection).
	 */
	class Section
	{
		public:
			Section();
			Section(const char *name);
			Section(const std::string &name);
			Section &operator=(const Section &sec);
			bool has(const char *key) const;
			bool has(const std::string &key) const;
			const char *get_or(const char *key, const char *def) const;
			const std::string &get_or(const std::string &key, const std::string &def) const;
			const std::string &operator[](const char *key) const;
			const std::string &operator[](const std::string &key) const;
			const std::string &get_name() const;
			const std::map<std::string, std::vector<std::string>> &get_entries() const;
			const std::vector<std::string> &get_all(const std::string &key) const;
			const std::vector<std::string> &get_all(const char *key) const;
			void push(const std::string &key, const std::string &val);
			void set(const std::string &key, size_t index, std::string &val);
			void create_key(const std::string &key);
		
		private:
			std::string name;
			std::map<std::string, std::vector<std::string>> entries;
	};

	typedef std::map<std::string, Section> Config;

	extern Config *load(const char *path);
	extern int save(const char *path, const Config *configuration);

	extern void dump_conf(const Config *conf);
	extern void dump_section(const Section &sect);

} }

#endif
