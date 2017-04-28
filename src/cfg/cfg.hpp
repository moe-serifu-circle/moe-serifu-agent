#ifndef MSA_CFG_CFG_HPP
#define MSA_CFG_CFG_HPP

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

namespace msa { namespace cfg {

	/**
	 * Use the config section like a collection of values (with get_all()), or you can just
	 * use it as though there is only one value per key (use the [] operator to get the
	 * first value in the collection).
	 */
	class Section
	{
		public:
			Section();
			Section(const std::string &name);
			Section &operator=(const Section &sec);
			bool has(const char *key) const;
			bool has(const std::string &key) const;
			const std::string &operator[](const std::string &key) const;
			const std::string &get_name() const;
			const std::map<std::string, std::vector<std::string>> &get_entries() const;
			const std::vector<std::string> &get_all(const std::string &key) const;
			const std::vector<std::string> &get_all(const char *key) const;
			void push(const std::string &key, const std::string &val);
			void set(const std::string &key, size_t index, std::string &val);
			void create_key(const std::string &key);
			template<class T> const T &get_as(const std::string &key) const
			{
				std::istringstream ss((*this)[key]);
				T typed;
				ss >> typed;
				return typed;
			}
			template<class T> const T &get_or(const std::string &key, const T &def) const
			{
				return has(key) ? get_as<T>(key) : def;
			}
		
		private:
			std::string name;
			std::map<std::string, std::vector<std::string>> entries;
	};

	class config_error : public std::runtime_error
	{
		public:
			config_error(const std::string &sec, const std::string &key, const std::string &what);
			virtual const char *what() const;
			const char *key() const;
			const char *sec() const;

		private:
			const std::string _key;
			const std::string _section;
	};

	typedef std::map<std::string, Section> Config;

	extern Config *load(const char *path);
	extern int save(const char *path, const Config *configuration);

	extern void dump_conf(const Config *conf);
	extern void dump_section(const Section &sect);

} }

#endif
