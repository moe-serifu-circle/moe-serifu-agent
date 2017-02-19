#include "thread.hpp"

#if defined(__WIN32)
	#include "win32.cpp"
#elif defined(__ANDROID__)
	#include "android.cpp"
#else
	#include "unix.cpp"
#endif
