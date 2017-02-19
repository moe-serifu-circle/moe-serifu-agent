// android threading. Uses pthreads, but fills in where some non-portable
// functions are missing

#include <map>

namespace msa { namespace thread {

	// holds thread-local information
	typedef struct info_type
	{
		char *name;
	} Info;

	typedef struct runner_arg_type
	{
		void *(*start_routine)(void *);
		void *start_routine_arg;
		Mutex *start_mutex;
	} RunnerArgs;

	static std::map<Thread, Info *> __info;

	static void __info_dispose(Info *info);
	static void __info_create(Info **info);
	static void *__run(void *arg);

	extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg)
	{
		RunnerArgs *ra = new RunnerArgs;
		ra->start_routine = start_routine;
		ra->start_routine_arg = arg;
		ra->start_mutex = new Mutex;
		if (mutex_init(ra->start_mutex, NULL) != 0)
		{
			delete ra->start_mutex;
			delete ra;
			return -1;
		}
		
		if (mutex_lock(ra->start_mutex) != 0)
		{
			mutex_destroy(ra->start_mutex);
			delete ra->start_mutex;
			delete ra;
			return -1;
		}
		int status = pthread_create(thread, attr, __run, ra);
		if (status != 0)
		{
			mutex_destroy(ra->start_mutex);
			delete ra->start_mutex;
			delete ra;
			return status;
		}
		Info *info;
		__info_create(&info);
		__info[*thread] = info;
		if (mutex_unlock(ra->start_mutex) != 0)
		{
			__info_dispose(info);
			mutex_destroy(ra->start_mutex);
			delete ra->start_mutex;
			delete ra;
			return -1;
		}
		return status;
	}
	
	extern int join(Thread thread, void **value_ptr)
	{
		return pthread_join(thread, value_ptr);
	}
		
	extern int set_name(Thread thread, const char *name)
	{
		int status = pthread_setname_np(thread, name);
		if (status != 0)
		{
			return status;
		}
		strncpy(__info[thread]->name, name, 15);
		__info[thread]->name[15] = '\0';
		return status;
	}
		
	extern int get_name(Thread thread, char *name, size_t len)
	{
		strncpy(name, __info[thread]->name, len - 1);
		name[len - 1] = '\0';
		return 0;
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

	static void *__run(void *arg)
	{
		RunnerArgs *ra = (RunnerArgs *) arg;
		void *(*start_routine)(void *) = ra->start_routine;
		void *start_routine_arg = ra->start_routine_arg;
		Mutex *start_mutex = ra->start_mutex;
		delete ra;
		
		mutex_lock(start_mutex);
		void *retval = start_routine(start_routine_arg);
		mutex_unlock(start_mutex);
		mutex_destroy(start_mutex);
		delete start_mutex;

		Info *info = __info[self()];
		__info_dispose(info);
		
		return retval;
	}

	static void __info_create(Info **info_ptr)
	{
		Info *info = new Info;
		info->name = new char[16];
		*info_ptr = info;
	}

	static void __info_dispose(Info *info)
	{
		delete[] info->name;
		delete info;
	}
} }
