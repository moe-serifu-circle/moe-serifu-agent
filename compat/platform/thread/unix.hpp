#ifndef COMPAT_PLATFORM_THREAD_THREAD_HPP
	#error "Do not include compat libs directly"
#endif

#include <pthread.h>

namespace msa { namespace thread {
		
	typedef pthread_t Thread;
	typedef pthread_attr_t Attributes;
	typedef pthread_mutex_t Mutex;
	typedef pthread_mutexattr_t MutexAttributes;
	typedef pthread_cond_t Cond;
	typedef pthread_condattr_t CondAttributes;

} }
