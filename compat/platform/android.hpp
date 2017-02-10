// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

#include <stdexcept>
#include <cctype>
#include <ctime>
#include <cstdint>

// Bionic is an extremely limited implementation of libc, and it is missing many functions. This
// file adds those functions to the std namespace, which will begin to pollute it, but as the
// function does not currently exist that's acceptable for now.

namespace std {
	
	inline static int stoi(const String &str, size_t *pos = 0, int base = 10)
	{
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
	}

}

// now we move on to the actual compat functions
namespace msa { namespace platform {

	static inline void sleep(int millisec)
	{
		struct timespec t;
		t.tv_nsec = (uint64_t) millisec * UINT64_C(1000000000);
		nanosleep(&t, NULL);
	}

} }
