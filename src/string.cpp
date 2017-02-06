#include "string.hpp"
#include <cctype>

namespace msa { namespace util {

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

} }
