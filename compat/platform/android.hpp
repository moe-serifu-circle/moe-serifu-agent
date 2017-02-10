// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

#include <string>
#include <stdexcept>
#include <cctype>
#include <ctime>
#include <cstdint>
#include <cstdio>

// Bionic is an extremely limited implementation of libc, and it is missing many functions. This
// file adds those functions to the std namespace, which will begin to pollute it, but as the
// function does not currently exist that's acceptable for now.

namespace std {
	
	inline static std::string to_string(int value)
	{
		char buf[64];
		sprintf(buf, "%d", value);
		return std::string(buf);
	}

	inline static std::string to_string(long value)
	{
		char buf[64];
		sprintf(buf, "%ld", value);
		return std::string(buf);
	}

	inline static std::string to_string(long long value)
	{
		char buf[64];
		sprintf(buf, "%lld", value);
		return std::string(buf);
	}

	inline static std::string to_string(unsigned value)
	{
		char buf[64];
		sprintf(buf, "%u", value);
		return std::string(buf);
	}

	inline static std::string to_string(unsigned long value)
	{
		char buf[64];
		sprintf(buf, "%lu", value);
		return std::string(buf);
	}

	inline static std::string to_string(unsigned long long value)
	{
		char buf[64];
		sprintf(buf, "%lld", value);
		return std::string(buf);
	}

	inline static std::string to_string(float value)
	{
		char buf[64];
		sprintf(buf, "%f", value);
		return std::string(buf);
	}

	inline static std::string to_string(double value)
	{
		char buf[64];
		sprintf(buf, "%f", value);
		return std::string(buf);
	}

	inline static std::string to_string(long double value)
	{
		char buf[64];
		sprintf(buf, "%Lf", value);
		return std::string(buf);
	}
	
	inline static int stoi(const std::string &str, size_t *pos = 0, int base = 10)
	{
		// first sanity check on args
		if (base < 0 || base == 1 || base > 36)
		{
			throw std::invalid_argument("base must be one of {0,2,3, ... 36}");
		}

		const char *cptr = str.c_str();

		// find location of first non-space char
		size_t idx = 0;
		while (isspace(cptr[idx]) && idx < str.size())
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
		if (cptr[idx] == '-' || cptr[idx] == '+')
		{
			neg = (cptr[idx] == '-') ? -1 : 1;
			idx++;
		}

		// if we are at end, throw
		if (idx == str.size())
		{
			throw std::invalid_argument("no valid digits: " + str);
		}
		
		// check for octal / hex prefix
		if (cptr[idx] == '0')
		{
			// check that the next char is X or x before running other checks
			bool hex_prefix = false;
			if (idx + 1 < str.size() && (cptr[idx + 1] == 'x' || cptr[idx + 1] == 'X'))
			{
				hex_prefix = true;
			}
				// now do the base checks
			if (base == 8 || (base == 16 && hex_prefix))
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
					base = 16;
					idx += 2;
				}
				else if (idx + 1 < str.size() && (cptr[idx + 1] >= '0' && cptr[idx + 1] <= '7'))
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
		int digit_val;
		int num_digits = 0;
		for (; idx < str.size(); idx++)
		{
			char ch = cptr[idx];
			
			// get the digit value
			if (ch >= 'a' && ch <= 'z')
			{
				digit_val = (int) (10 + (ch - 'a'));
			}
			else if (ch >= 'A' && ch <= 'Z')
			{
				digit_val = (int) (10 + (ch - 'A'));
			}
			else if (ch >= '0' && ch <= '9')
			{
				digit_val = (int) (ch - '0');
			}
			else
			{
				// invalid character, stop processing
				break;
			}

			if (digit_val >= base)
			{
				// invalid digit for our base, stop processing
				break;
			}

			// 'shift' all digits by multiplying current value by the base (check all vals for overflow)
			int multip = base * num_digits;
			
			// overflow check
			if (multip < 0)
			{
				throw std::out_of_range("value out of range: " + str);
			}			

			val *= multip;

			// overflow check
			if (val < 0)
			{
				throw std::out_of_range("value out of range: " + str);
			}
			
			val += digit_val;

			// overflow check
			if (val < 0)
			{
				throw std::out_of_range("value out of range: " + str);
			}

			num_digits++;
		}
		
		if (idx == start)
		{
			throw std::invalid_argument("no valid digits: " + str);
		}
		
		if (pos != 0)
		{
			*pos = idx;
		}

		val *= neg;

		// final overflow check
		if ((neg > 0 && val < 0) || (neg < 0 && val > 0))
		{
			throw std::out_of_range("value out of range: " + str);
		}

		return val;
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
