// unix threading. uses pthreads implementation

namespace msa { namespace thread {

	extern int init()
	{
		// nothing to do here
		return 0;
	}

	extern int quit()
	{
		// nothing to do here
		return 0;
	}

	extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg)
	{
		return pthread_create(thread, attr, start_routine, arg);
	}
	
	extern int join(Thread thread, void **value_ptr)
	{
		return pthread_join(thread, value_ptr);
	}
		
	extern int set_name(Thread thread, const char *name)
	{
		return pthread_setname_np(thread, name);
	}
		
	extern int get_name(Thread thread, char *name, size_t len)
	{
		return pthread_getname_np(thread, name, len);
	}

	extern Thread self()
	{
		return pthread_self();
	}
		
	extern int attr_init(Attributes *attr)
	{
		return pthread_attr_init(attr);
	}
		
	extern int attr_set_detach(Attributes *attr, bool detach)
	{
		int state = detach ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
		return pthread_attr_setdetachstate(attr, state);
	}
		
	extern int attr_get_detach(const Attributes *attr, bool *detach)
	{
		int state;
		int status = pthread_attr_getdetachstate(attr, &state);
		*detach = (state == PTHREAD_CREATE_DETACHED);
		return status;
	}
		
	extern int attr_destroy(Attributes *attr)
	{
		return pthread_attr_destroy(attr);
	}		
		
	extern int mutex_init(Mutex *mutex, const MutexAttributes *attr)
	{
		return pthread_mutex_init(mutex, attr);
	}
		
	extern int mutex_destroy(Mutex *mutex)
	{
		return pthread_mutex_destroy(mutex);
	}
		
	extern int mutex_lock(Mutex *mutex)
	{
		return pthread_mutex_lock(mutex);
	}
		
	extern int mutex_unlock(Mutex *mutex)
	{
		return pthread_mutex_unlock(mutex);
	}

	extern int cond_init(Cond *cond, const CondAttributes *attr)
	{
		return pthread_cond_init(cond, attr);
	}
		
	extern int cond_destroy(Cond *cond)
	{
		return pthread_cond_destroy(cond);
	}
		
	extern int cond_wait(Cond *cond, Mutex *mutex)
	{
		return pthread_cond_wait(cond, mutex);
	}
		
	extern int cond_broadcast(Cond *cond)
	{
		return pthread_cond_broadcast(cond);
	}

	extern int cond_signal(Cond *cond)
	{
		return pthread_cond_signal(cond);
	}
} }
