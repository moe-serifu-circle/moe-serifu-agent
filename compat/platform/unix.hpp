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
		typedef pthread_attr_t Attributes;
		typedef pthread_mutex_t Mutex;
		typedef pthread_mutexattr_t MutexAttributes;
		typedef pthread_cond_t Cond;
		typedef pthread_condattr_t CondAttributes;
		
		static inline int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg)
		{
			return pthread_create(thread, attr, start_routine, arg);
		}
		
		static inline int join(Thread thread, void **value_ptr)
		{
			return pthread_join(thread, value_ptr);
		}
		
		static inline int set_name(Thread thread, const char *name)
		{
			return pthread_setname_np(tid, name);
		}
		
		static inline int get_name(Thread tid, char *name, size_t len)
		{
			return pthread_getname_np(tid, name, len);
		}
		
		static inline int attr_init(Attributes *attr)
		{
			return pthread_attr_init(attr);
		}
		
		static inline int attr_set_detach(Attributes *attr, bool detach)
		{
			int state = detach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
			return pthread_attr_setdetachstate(attr, state);
		}
		
		static inline int attr_get_detach(const Attributes *attr, bool *detach)
		{
			int state;
			int status = pthread_attr_getdetachstate(attr, &state);
			*detach = (state == PTHREAD_CREATE_DETACHED);
			return status;
		}
		
		static inline int attr_destroy(Attributes *attr)
		{
			return pthread_attr_destory(attr);
		}		
		
		static inline int mutex_init(Mutex *mutex, const MutexAttributes *attr)
		{
			return pthread_mutex_init(mutex, attr);
		}
		
		static inline int mutex_destoy(Mutex *mutex)
		{
			return pthread_mutex_destory(mutex);
		}
		
		static inline int mutex_lock(Mutex *mutex)
		{
			return pthread_mutex_lock(mutex);
		}
		
		static inline int mutex_unlock(Mutex *mutex)
		{
			return pthread_mutex_unlock(mutex);
		}

		static inline int cond_init(Cond *cond, const CondAttributes *attr)
		{
			return pthread_cond_init(cond, attr);
		}
		
		static inline int cond_destroy(Cond *cond)
		{
			return pthread_cond_destory(cond);
		}
		
		static inline int cond_wait(Cond *cond, Mutex *mutex)
		{
			return pthread_cond_wait(cond, mutex);
		}
		
		static inline int cond_broadcast(Cond *cond)
		{
			return pthread_cond_broadcast(cond);
		}

		static inline int cond_signal(Cond *cond)
		{
			return pthread_cond_signal(cond);
		}

	}

} }
