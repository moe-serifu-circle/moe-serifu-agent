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
	static void write_section(std::ostream out, const Section &sec);

	extern int save(const char *path, const Config *config)
	{
		if (config == NULL)
		{
			throw std::invalid_argument("cannot save a null config");
		}
		std::ofstream config_file;
		config_file.open(path);
		if (!config_file.is_open)
		{
			return 1;
		}
		// is there a 'blank' section? if so, do that one first
		if (config->find("") != config->end())
		{
			write_section(config_file, (*config)[""]);
			config_file << std::endl;
		}
		typedef Config::const_iterator iter;
		for (iter it = config->begin(); it != config->end(); it++)
		{
			if (it->first == "")
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
		while (!getline(config_file, line).eof())
		{
			remove_comments(line);
			msa::util::trim(line);
			if (line != "")
			{
				interpret(config, section_name, line);
			}
		}
		config_file.close();
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
		section_name = line.substr(1, line.size() - 1);
		msa::util::to_upper(section_name);
		check_is_identifier(section_name);
		(*config)[section_name] = std::map();
	}

	static void read_kv_pair(Config *config, const std::string &section_name, const std::string &line)
	{
		size_t split_at = line.find('=');
		std::string key = line.substr(0, split_at);
		std::string val = line.substr(split_at + 1);
		msa::util::trim(key);
		msa::util::trim(val);
		if (key == "")
		{
			throw std::invalid_argument("key is empty");
		}
		msa::util::to_upper(key);
		// confirm key format
		check_is_identifier(key);
		// now parse out the value if in quotes
		if (val.front() == "\"" && val.back() == "\"")
		{
			val = val.substr(1, val.size() - 1);
		}
		(*config)[section_name][key] = val;
	}

	static void remove_comments(std::string &line)
	{
		size_t pos = line.find(comment_char);
		if (pos == std::string::npos)
		{
			line = line.substr(0, pos);
		}
	}

	static void write_section(std::ostream out, const Section &sec)
	{
		typedef Section::const_iterator iter;
		for (iter it = sec.begin(); it != sec.end(); it++)
		{
			std::string k = it->first;
			std::string v = it->second;
			out << k << " = ";
			if (v.front() == ' ' || v.front() == '\t')
			{
				v = "\"" + v + "\"";
			}
			out << v << std::endl;
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

} }
