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

	static inline bool select_stdin()
	{
		HANDLE stdin = GetStandardHandle(STD_INPUT_HANDLE);
		return (WaitForSingleObject(stdin, 0) == WAIT_OBJECT_0);
	}

} }
