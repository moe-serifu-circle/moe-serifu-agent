#ifndef MSA_STRING_HPP
#define MSA_STRING_HPP

#include <string>

#ifdef __ANDROID__
	// android needs these in order to roll own stoi
	#include <stdexcept>
	#include <cctype>
#endif

namespace msa { namespace util {

	typedef std::string String;

	extern const String default_ws;

	extern String &left_trim(String &str, const String &search = default_ws);
	extern String &right_trim(String &str, const String &search = default_ws);
	extern String &trim(String &str, const String &search = default_ws);
	extern String &to_upper(String &str);
	extern String &to_lower(String &str);
	
	// define this for compatibility with bionic in android ndk
	inline static int stoi(const String &str, size_t *pos = 0, int base = 10)
	{
		#ifndef __ANDROID__
			// just pass this on up to the real call
			return std::stoi(str, pos, base);
		#else
			// Due to minimalistic libc (bionic), android does not support std::stoi().
			// we will therefor roll our own.

			// first sanity check on args
			if (base < 0 || base == 1 || base > 36)
			{
				throw std::invalid_argument("base must be one of {0,2,3, ... 36}, but was " + std::to_string(base));
			}

			// find location of first non-space char
			size_t idx = 0;
			while (isspace(str.c_str()[idx]) && idx < str.size())
			{
				idx++;
			}

			// if we found no valid chars, throw
			if (idx == str.size())
			{
				throw std::invalid_argument("no valid digits: " + str);
			}
			
			// check for negative / positive
			int neg = 1;
			if (str.c_str[idx] == '-' || str.c_str[idx] == '+')
			{
				neg = (str.c_str[idx] == '-') ? -1 : 1;
				idx++;
			}

			// if we are at end, throw
			if (idx == str.size())
			{
				throw std::invalid_argument("no valid digits: " + str);
			}
			
			// check for octal / hex prefix
			if (str.c_str[idx] == '0')
			{
				// check that the next char is X or x before running other checks
				bool hex_prefix = false;
				if (idx + 1 < str.size() && (str.c_str[idx + 1] == 'x' || str.c_str[idx + 1] == 'X'))
				{
					hex_prefix = true;
				}

				// now do the base checks
				if (base == 8 || base == 16 && hex_prefix)
				{
					idx++;
				}
				else if (base == 16)
				{
					idx += 2;
				}
				else if (base == 0)
				{
					if (hex_prefix)
					{
						base == 16;
						idx += 2;
					}
					else if (idx + 1 < str.size() && (str.c_str[idx + 1] >= '0' && str.c_str[idx + 1] <= '7'))
					{
						base = 8;
						idx++;
					}
				}
			} else if (base == 0) {
				base = 10;
			}
			

			// if we are at end, throw
			if (idx == str.size())
			{
				throw std::invalid_argument("no valid digits: " + str);
			}
			
			// okay, now actually iterate through and get the value
			int val = 0;
			size_t start = idx;
			for (; idx < str.size(); idx++)
			{
				
			}
			
			if (idx == start)
			{
				throw std::invalid_argument("no valid digits: " + str);
			}

			if (pos != 0)
			{
				*pos = idx;
			}

			return val * neg;
		#endif
	}

} }

#endif
