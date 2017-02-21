#ifndef COMPAT_PLATFORM_THREAD_THREAD_HPP
#define COMPAT_PLATFORM_THREAD_THREAD_HPP

#if defined(__WIN32)
	#include "win32.hpp"
#elif defined(__ANDROID__)
	#include "android.hpp"
#else
	#include "unix.hpp"
#endif

namespace msa { namespace thread {
	
	extern int init();
	extern int quit();
	extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg, const char *name);
	extern int join(Thread thread, void **value_ptr);
	extern int set_name(Thread thread, const char *name);
	extern int get_name(Thread thread, char *name, size_t len);
	extern Thread self();
		
	extern int attr_init(Attributes *attr);
	extern int attr_set_detach(Attributes *attr, bool detach);
	extern int attr_get_detach(const Attributes *attr, bool *detach);
	extern int attr_destroy(Attributes *attr);
		
	extern int mutex_init(Mutex *mutex, const MutexAttributes *attr);
	extern int mutex_destroy(Mutex *mutex);
	extern int mutex_lock(Mutex *mutex);
	extern int mutex_unlock(Mutex *mutex);

	extern int cond_init(Cond *cond, const CondAttributes *attr);
	extern int cond_destroy(Cond *cond);
	extern int cond_wait(Cond *cond, Mutex *mutex);
	extern int cond_broadcast(Cond *cond);
	extern int cond_signal(Cond *cond);

} }

#endif
