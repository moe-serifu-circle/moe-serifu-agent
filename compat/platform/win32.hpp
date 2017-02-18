// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

extern "C" {
	#include <windows.h>
}

#include <map>
#include <cstring>
#include <ctime>

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

	namespace thread {

		typedef DWORD Thread;
		typedef struct attr_type Attributes;
		typedef struct mutex_type Mutex;
		typedef struct mutex_attr_type MutexAttributes;
		typedef struct cond_type Cond;
		typedef struct cond_attr_type CondAttributes;
		
		
		extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg);
		extern int join(Thread thread, void **value_ptr);
		extern int set_name(Thread thread, const char *name);
		extern int get_name(Thread tid, char *name, size_t len);
		
		extern int attr_init(Attributes *attr);
		extern int attr_set_detach(Attributes *attr, bool detach);
		extern int attr_get_detach(const Attributes *attr, bool *detach);
		extern int attr_destroy(Attributes *attr);
		
		extern int mutex_init(Mutex *mutex, const MutexAttributes *attr);
		extern int mutex_destoy(Mutex *mutex);
		extern int mutex_lock(Mutex *mutex);
		extern int mutex_unlock(Mutex *mutex);

		extern int cond_init(Cond *cond, const CondAttributes *attr);
		extern int cond_destroy(Cond *cond);
		extern int cond_wait(Cond *cond, Mutex *mutex);
		extern int cond_broadcast(Cond *cond);
		extern int cond_signal(Cond *cond);

	}

} }
