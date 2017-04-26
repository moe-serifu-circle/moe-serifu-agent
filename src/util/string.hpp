#ifndef MSA_UTIL_STRING_HPP
#define MSA_UTIL_STRING_HPP

#include <string>
#include <vector>

namespace msa { namespace string {

	typedef std::string String;

	extern const String default_ws;

	extern String &left_trim(String &str, const String &search = default_ws);
	extern String &right_trim(String &str, const String &search = default_ws);
	extern String &trim(String &str, const String &search = default_ws);
	extern String &to_upper(String &str);
	extern String &to_lower(String &str);
	extern void tokenize(const String &str, char separator, std::vector<String> &output);
	extern bool ends_with(const String &str, const String &suffix);
	extern bool starts_with(const String &str, const String &prefix);

} }

#endif
