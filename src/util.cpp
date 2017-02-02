#include "util.hpp"

#ifdef _WIN32
	// windows includes
	extern "C" {
		#include <windows.h>
	}
#else
	// unix includes
	#include <ctime>
	#include <cstdint>
#endif

namespace msa { namespace util {

#ifdef _WIN32
	extern void sleep_milli(int millisec) {
		Sleep(millisec);
	}
#else
	extern void sleep_milli(int millisec) {
		struct timespec t;
		t.tv_nsec = (uint64_t) millisec * UINT64_C(1000000000);
		nanosleep(&t, NULL);
	}
#endif

} }
