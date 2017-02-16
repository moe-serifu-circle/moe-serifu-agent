// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

// This file is compatible with unix and linux systems; really anything that tries to
// to follow the posix standard

#include <ctime>
#include <cstdint>

#include <sys/select.h>
#include <pthread.h>

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

		static inline int set_name(Thread tid, const char *name)
		{

		}

		static inline int get_name(Thread tid, char *name, size_t len)
		{
			
		}

	}

} }
