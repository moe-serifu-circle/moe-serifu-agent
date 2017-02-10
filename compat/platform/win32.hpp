// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

extern "C" {
	#include <windows.h>
}

namespace msa { namespace platform {

	static inline void sleep(int millisec)
	{
		Sleep(millisec);
	}

} }