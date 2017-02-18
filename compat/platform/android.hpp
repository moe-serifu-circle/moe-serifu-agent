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

#include <sys/select.h>

#include <pthread.h>

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
	
	static inline bool select_stdin()
	{
		static fd_set fds;
		static struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		select(1, &fds, NULL, NULL, &tv);
		return FD_ISSET(0, &fds);
	}

	namespace thread {
		
		typedef pthread_t Thread;
		typedef pthread_attr_t Attributes;
		typedef pthread_mutex_t Mutex;
		typedef pthread_mutexattr_t MutexAttributes;
		typedef pthread_cond_t Cond;
		typedef pthread_condattr_t CondAttributes;
		
		static inline int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg)
		{
			return pthread_create(thread, attr, start_routine, arg);
		}
		
		static inline int join(Thread thread, void **value_ptr)
		{
			return pthread_join(thread, value_ptr);
		}
		
		static inline int set_name(Thread thread, const char *name)
		{
			return pthread_setname_np(tid, name);
		}
		
		static inline int get_name(Thread tid, char *name, size_t len)
		{
			if (len < 16)
			{
				return 1;
			}
			return prctl(PR_GET_NAME, name);
		}
		
		static inline int attr_init(Attributes *attr)
		{
			return pthread_attr_init(attr);
		}
		
		static inline int attr_set_detach(Attributes *attr, bool detach)
		{
			int state = detach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
			return pthread_attr_setdetachstate(attr, state);
		}
		
		static inline int attr_get_detach(const Attributes *attr, bool *detach)
		{
			int state;
			int status = pthread_attr_getdetachstate(attr, &state);
			*detach = (state == PTHREAD_CREATE_DETACHED);
			return status;
		}
		
		static inline int attr_destroy(Attributes *attr)
		{
			return pthread_attr_destory(attr);
		}		
		
		static inline int mutex_init(Mutex *mutex, const MutexAttributes *attr)
		{
			return pthread_mutex_init(mutex, attr);
		}
		
		static inline int mutex_destoy(Mutex *mutex)
		{
			return pthread_mutex_destory(mutex);
		}
		
		static inline int mutex_lock(Mutex *mutex)
		{
			return pthread_mutex_lock(mutex);
		}
		
		static inline int mutex_unlock(Mutex *mutex)
		{
			return pthread_mutex_unlock(mutex);
		}

		static inline int cond_init(Cond *cond, const CondAttributes *attr)
		{
			return pthread_cond_init(cond, attr);
		}
		
		static inline int cond_destroy(Cond *cond)
		{
			return pthread_cond_destory(cond);
		}
		
		static inline int cond_wait(Cond *cond, Mutex *mutex)
		{
			return pthread_cond_wait(cond, mutex);
		}
		
		static inline int cond_broadcast(Cond *cond)
		{
			return pthread_cond_broadcast(cond);
		}

		static inline int cond_signal(Cond *cond)
		{
			return pthread_cond_signal(cond);
		}

	}

} }
