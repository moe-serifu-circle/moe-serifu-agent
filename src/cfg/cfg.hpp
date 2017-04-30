#ifndef MSA_CFG_CFG_HPP
#define MSA_CFG_CFG_HPP

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <type_traits>

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
			bool has(const std::string &key) const;
			const std::string &operator[](const std::string &key) const;
			const std::string &get_name() const;
			const std::map<std::string, std::vector<std::string>> &get_entries() const;
			const std::vector<std::string> &get_all(const std::string &key) const;
			void push(const std::string &key, const std::string &val);
			void set(const std::string &key, size_t index, std::string &val);
			void create_key(const std::string &key);

			/**
			 * Checks if a key exists, and throws a config_error if it doesn't.
			 */
			void check_exists(const std::string &key) const;

			/**
			 * Checks if a key exists, and that the value falls within the given range (inclusive).
			 *
			 * <T> must be an arithmetic fundamental type.
			 */
			template<class T> void check_range(const std::string &key, const T &min, const T &max) const
			{
				T typed = get_as<T>(key);
				if (typed < min)
				{
					throw config_error(get_name(), key, "must be greater than or equal to " + std::to_string(min));
				}
				if (typed > max)
				{
					throw config_error(get_name(), key, "must be less than or equal to " + std::to_string(min));
				}
			}

			/**
			 * Checks if a key exists, and that all values fall within the given range (inclusive).
			 *
			 * <T> must be an arithmetic fundamental type.
			 */
			template<class T> void check_range_all(const std::string &key, const T &min, const T &max) const
			{
				std::vector<T> items;
				get_all_as<T>(key, items);
				for (size_t i = 0; i < items.size(); i++)
				{
					if (items.at(i) < min)
					{
						throw config_error(get_name(), key, i, "must be greater than or equal to " + std::to_string(min));
					}
					if (items.at(i) > max)
					{
						throw config_error(get_name(), key, i, "must be less than or equal to " + std::to_string(min));
					}
				}
			}

			/**
			 * Gets the first index of a key and converts it to the given type.
			 *
			 * <T> must be an arithmetic fundamental type.
			 */
			template<class T> const T &get_as(const std::string &key) const
			{
				static_assert(std::is_artithmetic<T>::value, "for converting values, only arithmetic types are supported");
				std::istringstream ss((*this)[key]);
				T typed;
				ss >> typed;
				return typed;
			}

			/**
			 * Gets the first index of a key if it exists. The value is converted to
			 * the given type. If the key does not exist, the default value is returned.
			 *
			 * <T> must be an arithmetic fundamental type.
			 */
			template<class T> const T &get_or(const std::string &key, const T &def) const
			{
				return has(key) ? get_as<T>(key) : def;
			}

			/**
			 * Gets all values of a key. Each value is converted to the given type.
			 *
			 * <T> must be an arithmetic fundamental type.
			 */
			template<class T> void get_all_as(const std::string &key, std::vector<T> &items) const
			{
				static_assert(std::is_artithmetic<T>::value, "for converting values, only arithmetic types are supported");
				const std::vector<std::string> all = get_all(key);
				for (auto i = all.begin(); i != all.end(); i++)
				{
					std::istringstream ss(*i);
					T typed;
					ss >> typed;
					items.push_back(typed);
				}
			}
		
		private:
			std::string name;
			std::map<std::string, std::vector<std::string>> entries;
	};

	class config_error : public std::runtime_error
	{
		public:
			config_error(const std::string &sec, const std::string &key, const std::string &what);
			config_error(const std::string &sec, const std::string &key, size_t index, const std::string &what);
			virtual const char *what() const;
			const char *key() const;
			const char *section() const;
			size_t index() const;

		private:
			const std::string _key;
			const std::string _section;
			const size_t _index;
			const bool _explicit_index;
			mutable std::string _what_cache;
	};

	typedef std::map<std::string, Section> Config;

	extern Config *load(const char *path);
	extern int save(const char *path, const Config *configuration);

	extern void dump_conf(const Config *conf);
	extern void dump_section(const Section &sect);

} }

#endif
