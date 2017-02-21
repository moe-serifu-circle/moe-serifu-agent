// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

// This file is compatible with unix and linux systems; really anything that tries to
// to follow the posix standard

#include <ctime>
#include <cstdint>

#include <sys/select.h>

namespace msa { namespace platform {

	static inline void sleep(int millisec)
	{
		struct timespec t;
		t.tv_sec = millisec / 1000;
		t.tv_nsec = (uint64_t) (millisec % 1000) * UINT64_C(1000000000);
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
	
} }

