#include "util/string.hpp"
#include <cctype>

namespace msa { namespace string {

	const String default_ws = " \t\r\n";

	extern String &left_trim(String &str, const String &search)
	{
		if (str == "" || search == "")
		{
			return str;
		}
		size_t p = str.find_first_not_of(search);
		if (p == String::npos)
		{
			// it is entirely the string, return a blank one
			str = "";
			return str;
		}
		str = str.substr(p);
		return str;
	}

	extern String &right_trim(String &str, const String &search)
	{
		if (str == "" || search == "")
		{
			return str;
		}
		size_t p = str.find_last_not_of(search);
		if (p == String::npos)
		{
			str = "";
			return str;
		}
		str = str.substr(0, p + 1);
		return str;
	}

	extern String &trim(String &str, const String &search)
	{
		if (str == "" || search == "")
		{
			return str;
		}
		left_trim(str, search);
		right_trim(str, search);
		return str;
	}

	extern String &to_upper(String &str)
	{
		for (auto &c : str)
		{
			c = (char) std::toupper(c);
		}
		return str;
	}

	extern String &to_lower(String &str)
	{
		for (auto &c : str)
		{
			c = (char) std::tolower(c);
		}
		return str;
	}
	
	extern void tokenize(const String &str, char separator, std::vector<String> &output)
	{
		String cur_token;
		for (size_t i = 0; i < str.size(); i++)
		{
			if (str[i] != separator)
			{
				cur_token.push_back(str[i]);
			}
			else if (cur_token.size() > 0)
			{
				output.push_back(cur_token);
				cur_token = "";
			}
		}
		if (cur_token.size() > 0)
		{
			output.push_back(cur_token);
		}
	}
	
	extern bool ends_with(const String &str, const String &suffix)
	{
		if (str.size() < suffix.size())
		{
			return false;
		}
		size_t start_pos = str.size() - suffix.size();
		return (str.compare(start_pos, String::npos, suffix) == 0);
	}

	extern bool starts_with(const String &str, const String &prefix)
	{	
		if (str.size() < prefix.size())
		{
			return false;
		}
		return (str.compare(0, prefix.size(), prefix) == 0);
	}

} }

