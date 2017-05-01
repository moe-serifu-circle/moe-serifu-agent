#include "cfg/cfg.hpp"
#include "util/string.hpp"

#include <fstream>
#include <stdexcept>

namespace msa { namespace cfg {

	const char section_header_start_char = '[';
	const char section_header_end_char = ']';
	const char comment_char = '#';

	static void interpret(Config *config, std::string &section_name, const std::string &line);
	static void read_section_header(Config *config, std::string &section_name, const std::string &line);
	static void read_kv_pair(Config *config, const std::string &section_name, const std::string &line);
	static void check_is_identifier(const std::string &check);
	static void remove_comments(std::string &line);
	static void write_section(std::ostream &out, const Section &sec);

	extern void dump_conf(const Config *conf)
	{
		std::map<std::string, Section>::const_iterator it;
		for (it = conf->begin(); it != conf->end(); it++)
		{
			printf("%s:\n", it->first.c_str());
			dump_section(it->second);
			printf("\n");
		}
	}

	extern void dump_section(const Section &sect)
	{
		const std::map<std::string, std::vector<std::string>> entries = sect.get_entries();
		std::map<std::string, std::vector<std::string>>::const_iterator it;
		for (it = entries.begin(); it != entries.end(); it++)
		{
			printf("\t%s:\n", it->first.c_str());
			std::vector<std::string>::const_iterator it2;
			for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				printf("\t\t%s\n", it2->c_str());
			}
			printf("\n");
		}
	}

	extern int save(const char *path, const Config *config)
	{
		if (config == NULL)
		{
			throw std::invalid_argument("cannot save a null config");
		}
		std::ofstream config_file;
		config_file.open(path);
		if (!config_file.is_open())
		{
			return 1;
		}
		// is there a 'blank' section? if so, do that one first
		
		typedef Config::const_iterator iter;
		iter it = config->find(std::string(""));
		if (it != config->end())
		{
			write_section(config_file, it->second);
			config_file << std::endl;
		}
		for (iter it = config->begin(); it != config->end(); it++)
		{
			if (it->first == std::string(""))
			{
				continue;
			}
			config_file << section_header_start_char << it->first << section_header_end_char << std::endl;
			write_section(config_file, it->second);
			config_file << std::endl;
		}
		config_file.close();
		return 0;
	}

	extern Config *load(const char *path)
	{
		std::ifstream config_file;
		config_file.open(path);
		Config *config = NULL;
		if (!config_file.is_open())
		{
			return config;
		}
		std::string line;
		std::string section_name = "";
		config = new Config;
		int line_num = 1;
		bool no_errors = true;
		while (!getline(config_file, line).eof())
		{
			remove_comments(line);
			msa::string::trim(line);
			if (line != "")
			{
				// parsing is all-or-nothing, but try to parse the whole thing
				// so all errors are shown at once
				try
				{
					interpret(config, section_name, line);
				}
				catch (std::exception &e)
				{
					printf("error parsing config file:\n");
					printf("  on line %d: %s\n", line_num, e.what());
					no_errors = false;
				}
			}
			line_num++;
		}
		config_file.close();
		if (!no_errors)
		{
			printf("\ncould not parse config file '%s'\n", path);
			delete config;
			config = NULL;
		}
		return config;
	}
	
	static void interpret(Config *config, std::string &section_name, const std::string &line)
	{
		const char &start = section_header_start_char;
		const char &end = section_header_end_char;
		if (line.front() == start && line.back() == end)
		{
			read_section_header(config, section_name, line);
		}
		else if (line.find('=') != std::string::npos)
		{
			read_kv_pair(config, section_name, line);
		}
		else
		{
			throw std::invalid_argument("line is not in proper format");
		}
	}

	static void read_section_header(Config *config, std::string &section_name, const std::string &line)
	{
		section_name = line.substr(1, line.size() - 2);
		msa::string::to_upper(section_name);
		check_is_identifier(section_name);
		(*config)[section_name] = Section(section_name);
	}

	static void read_kv_pair(Config *config, const std::string &section_name, const std::string &line)
	{
		// split the line at the equals sign
		size_t split_at = line.find('=');
		std::string key = line.substr(0, split_at);
		std::string val = line.substr(split_at + 1);
		msa::string::trim(key);
		msa::string::trim(val);
		
		// check if there is an index
		int index = -1;
		size_t bracket_pos = key.find('[');
		if (bracket_pos != std::string::npos && key.back() == ']')
		{
			std::string idx_str = key.substr(bracket_pos + 1, key.size() - bracket_pos - 2);
			msa::string::trim(idx_str);
			// make sure we have ONLY digits
			if (idx_str.find_first_not_of("1234567890") != std::string::npos)
			{
				throw std::invalid_argument("key index can only be digits");
			}
			index = std::stoi(idx_str);
			key = key.substr(0, bracket_pos);
			msa::string::trim(key);
		}

		// sanity check on the key
		if (key == "")
		{
			throw std::invalid_argument("key cannot be blank");
		}
		msa::string::to_upper(key);
		// confirm key format
		check_is_identifier(key);
		
		// are there quotes around the value? take them out if so
		if (val.front() == '"' && val.back() == '"')
		{
			val = val.substr(1, val.size() - 2);
		}

		Section &sec = (*config)[section_name];
		sec.create_key(key);

		// did we have an explicit index set? If so, we must use it to set the
		// the value
		if (index != -1)
		{
			size_t idx = (size_t) index;
			while (sec.get_all(key).size() < idx + 1)
			{
				sec.push(key, std::string());
			}
			sec.set(key, idx, val);
		}
		else
		{
			sec.push(key, val);
		}
	}

	static void remove_comments(std::string &line)
	{
		size_t pos = line.find(comment_char);
		if (pos != std::string::npos)
		{
			line = line.substr(0, pos);
		}
	}

	static void write_section(std::ostream &out, const Section &sec)
	{
		typedef std::map<std::string, std::vector<std::string>> Entries;
		typedef Entries::const_iterator iter;
		const Entries ent = sec.get_entries();
		for (iter it = ent.begin(); it != ent.end(); it++)
		{
			std::string k = it->first;
			bool include_index = (it->second.size() > 1);
			int index = 0;
			std::vector<std::string>::const_iterator v;
			for (v = it->second.begin(); v != it->second.end(); v++)
			{
				out << k;
				if (include_index)
				{
					out << '[' << index << ']';
				}
				out << " = ";
				if (v->front() == ' ' || v->front() == '\t')
				{
					out << "\"" << *v << "\"" << std::endl;
				}
				else
				{
					out << *v << std::endl;
				}
				index++;
			}
		}
	}

	static void check_is_identifier(const std::string &str)
	{
		auto first = str.front();
		if (first >= '0' && first <= '9')
		{
			throw std::invalid_argument("identifiers cannot begin with a digit");
		}
		for (const auto &c : str)
		{
			if (c != '$' && c != '_' && (c < 'A' || c > 'Z') && (c < '0' || c > '9'))
			{
				throw std::invalid_argument("identifiers may only have the characters '_', '$', A-Z, and 0-9");
			}
		}
	}

	Section::Section() : name("") {}

	Section::Section(const std::string &name) : name(name) {}

	bool Section::has(const std::string &key) const
	{
		return entries.find(key) != entries.end();
	}

	const std::string &Section::operator[](const std::string &key) const
	{
		return get_all(key).at(0);
	}

	Section &Section::operator=(const Section &sec)
	{
		name = sec.name;
		entries = sec.entries;
		return *this;
	}

	const std::string &Section::get_name() const
	{
		return name;
	}

	const std::map<std::string, std::vector<std::string>> &Section::get_entries() const
	{
		return entries;
	}

	const std::vector<std::string> &Section::get_all(const std::string &key) const
	{
		check_exists(key);
		return entries.at(key);
	}

	void Section::push(const std::string &key, const std::string &val)
	{
		entries.at(key).push_back(val);
	}
	
	void Section::set(const std::string &key, size_t index, std::string &val)
	{
		entries.at(key)[index] = val;
	}
	
	void Section::create_key(const std::string &key)
	{
		if (!has(key))
		{
			entries[key] = std::vector<std::string>();
		}
	}

	void Section::check_exists(const std::string &key) const
	{
		if (!has(key))
		{
			throw config_error(get_name(), key, "", "key does not exist");
		}
	}

	config_error::config_error(const std::string &sec, const std::string &key, const std::string &val, const std::string &what) :
		runtime_error(what),
		_key(key),
		_section(sec),
		_index(0),
		_value(val),
		_explicit_index(false),
		_what_cache()
	{}

	config_error::config_error(const std::string &sec, const std::string &key, size_t index, const std::string &val, const std::string &what) :
		runtime_error(what),
		_key(key),
		_section(sec),
		_index(index),
		_value(val),
		_explicit_index(true),
		_what_cache()
	{}

	const char *config_error::what() const throw()
	{
		if (_what_cache.empty())
		{
			_what_cache = std::string(section()) + "." + key();
			if (_explicit_index)
			{
				_what_cache += std::string("[") + std::to_string(index()) + std::string("]");
			}
			_what_cache += std::string(" \"") + value() + "\" " + runtime_error::what();
		}
		return _what_cache.c_str();
	}

	const char *config_error::key() const
	{
		return _key.c_str();
	}
	
	const char *config_error::section() const
	{
		return _section.c_str();
	}

	const char *config_error::value() const
	{
		return _value.c_str();
	}

	size_t config_error::index() const
	{
		return _index;
	}

} }
