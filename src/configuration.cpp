#include "configuration.hpp"
#include "string.hpp"

#include <fstream>
#include <stdexcept>

namespace msa { namespace config {

	const char section_header_start_char = '[';
	const char section_header_end_char = ']';
	const char comment_char = '#';

	static void interpret(Config *config, std::string &section_name, const std::string &line);
	static void read_section_header(Config *config, std::string &section_name, const std::string &line);		static void read_kv_pair(Config *config, const std::string &section_name, const std::string &line);
	static void check_is_identifier(const std::string &check);
	static void remove_comments(std::string &line);
	static void write_section(std::ostream &out, const Section &sec);

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
			msa::util::trim(line);
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
		msa::util::to_upper(section_name);
		check_is_identifier(section_name);
		(*config)[section_name] = Section(section_name);
	}

	static void read_kv_pair(Config *config, const std::string &section_name, const std::string &line)
	{
		// split the line at the equals sign
		size_t split_at = line.find('=');
		std::string key = line.substr(0, split_at);
		std::string val = line.substr(split_at + 1);
		msa::util::trim(key);
		msa::util::trim(val);
		
		// check if there is an index
		int index = -1;
		size_t bracket_pos = key.find('[');
		if (bracket_pos != std::string::npos && key.back() == ']')
		{
			std::string idx_str = key.substr(bracket_pos + 1, key.size() - bracket_pos - 2);
			msa::util::trim(idx_str);
			// make sure we have ONLY digits
			if (idx_str.find_first_not_of("1234567890") != std::string::npos)
			{
				throw std::invalid_argument("key index can only be digits");
			}
			index = std::stoi(idx_str);
			key = key.substr(0, bracket_pos);
			msa::util::trim(key);
		}

		// sanity check on the key
		if (key == "")
		{
			throw std::invalid_argument("key cannot be blank");
		}
		msa::util::to_upper(key);
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

	Section::Section(const char *name) : name(name) {}

	Section::Section(const std::string &name) : name(name) {}

	bool Section::has(const char *key) const
	{
		const std::string str = std::string(key);
		return has(str);
	}

	bool Section::has(const std::string &key) const
	{
		return entries.find(key) != entries.end();
	}

	const char *Section::get_or(const char *key, const char *def) const
	{
		return has(key) ? (*this)[key].c_str() : def;
	}

	const std::string &Section::get_or(const std::string &key, const std::string &def) const
	{
		return has(key) ? (*this)[key] : def;
	}

	const std::string &Section::operator[](const char *key) const
	{
		return get_all(key).at(0);
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

	const std::vector<std::string> &Section::get_all(const char *key) const
	{
		std::string str = std::string(key);
		return get_all(str);
	}

	const std::vector<std::string> &Section::get_all(const std::string &key) const
	{
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

} }
