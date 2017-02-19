#ifndef COMPAT_PLATFORM_THREAD_THREAD_HPP
	#error "Do not include compat libs directly"
#endif

// this file is for win32 specific thread definitions

extern "C" {
	#include <windows.h>
}

namespace msa { namespace thread {

	typedef DWORD Thread;
	typedef struct attr_type Attributes;
	typedef struct mutex_type Mutex;
	typedef struct mutex_attr_type MutexAttributes;
	typedef struct cond_type Cond;
	typedef struct cond_attr_type CondAttributes;

} }	
