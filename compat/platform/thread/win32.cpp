// win32 threading. This follows the standard interface of the msa::thread
// module, but is not very efficient

#include <map>
#include <queue>
#include <unordered_set>
#include <string>

namespace msa { namespace thread {

	typedef struct attr_type {
		LPSECURITY_ATTRIBUTES sec,
		bool detach;
	};
	
	typedef struct mutex_type
	{
		HANDLE handle;
		MutexAttributes *attr;
	};

	typedef struct mutex_attr_type
	{
		void *placeholder;
	};

	typedef struct cond_type
	{
		Mutex *external_mutex;
		Mutex *internal_mutex;
		std::queue<Thread> wait_queue;
		std::unordered_set<Thread> threads;
		CondAttributes *attr;
	};

	typedef struct cond_attr_type
	{
		void *placeholder;
	};
		
	typedef struct info_type
	{
		HANDLE handle;
		volatile bool waiting_on_cond;
		char *name;
		bool joinable;
	} Info;

	typedef struct runner_args_type
	{
		void *start_routine_arg;
		void *(*start_routine)(void *);
		HANDLE start_mutex;
	} RunnerArgs;
		
	static DWORD __run(void *arg);
	static void __create_info(Thread thread, HANDLE handle, bool joinable);
	static void __destroy_info(Thread thread);
		
	static std::map<Thread, Info *> __info;
	static std::map<Thread, void *> __ret_values;
	static Thread main_thread_id;
	static bool inited = false;

	extern int init()
	{
		Thread tid = self();
		// create info for the main thread
		if (__info.find(tid) == __info.end())
		{
			__create_info(tid, GetCurrentThread(), false);
			set_name(tid, "main");
			main_thread_id = tid;
			inited = true;
		}
		return 0;
	}

	extern int quit()
	{
		if (inited)
		{
			Thread tid = main_thread_id;
			// delete info for the main thread
			if (__info.find(tid) != __info.end())
			{
				__destroy_info(tid);
			}
		}
		return 0;
	}

	extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg, const char *name)
	{
		LPSECURITY_ATTRIBUTES sec = NULL;
		bool joinable = false;
		if (attr != NULL) {
			sec = attr->sec;
			joinable = !attr->detach;
		}
		
		HANDLE start_mutex = CreateMutex(NULL, false, NULL);
		if (start_mutex == NULL)
		{
			return 1;
		}
		if (WaitForSingleObject(start_mutex, INFINITE) != WAIT_OBJECT_0)
		{
			CloseHandle(start_mutex);
			return 1;
		}
		
		RunnerArgs *ra = new RunnerArgs;
		ra->arg = arg;
		ra->start_routine = start_routine;
		ra->start_mutex = start_mutex;
		
		HANDLE thread_handle = CreateThread(sec, 0, __run, ra, 0, thread);
		if (thread_handle == NULL)
		{
			delete ra;
			ReleaseMutex(start_mutex);
			CloseHandle(start_mutex);
			return 1;
		}

		__create_info(*thread, thread_handle, joinable);

		if (name != NULL)
		{
			set_name(*thread, name);
		}

		ReleaseMutex(start_mutex);
		return 0;
	}

	extern int join(Thread thread, void **value_ptr)
	{
		if (WaitForSingleObject(__info[thread]->thread_handle, INFINITE) != WAIT_OBJECT_0)
		{
			return 1;
		}
		if (value_ptr != NULL)
		{
			*value_ptr = __ret_values[thread];
		}
		__ret_values.erase(thread);
		__destroy_info(thread);
		return 0;
	}

	extern int set_name(Thread thread, const char *name)
	{
		Thread tid = thread;
		if (__info.find(tid) == __info.end())
		{
			return 1;
		}
		strncpy(__info[tid]->name, name, 15);
		__info[tid]->name[15] = '\0';
	}

	extern int get_name(Thread thread, char *name, size_t len)
	{
		Thread tid = thread;
		if (__info.find(tid) == __info.end())
		{
			return 1;
		}
		strncpy(name, __info[tid]->names, len);
	}

	extern Thread self()
	{
		return GetCurrentThreadId();
	}
		
	extern int attr_init(Attributes *attr)
	{
		attr->sec = NULL;
		attr->detach = false;
		return 0;
	}
		
	extern int attr_set_detach(Attributes *attr, bool detach)
	{
		attr->detach = detach;
		return 0;
	}

	extern int attr_get_detach(const Attributes *attr, bool *detach)
	{
		*detach = attr->detach;
		return 0;
	}
		
	extern int attr_destroy(Attributes *attr)
	{
		// do nothing
		return 0;
	}
		
	extern int mutex_init(Mutex *mutex, const MutexAttributes *attr)
	{
		mutex->handle = CreateMutex(NULL, false, NULL);
		if (mutex->handle == NULL)
		{
	 		return 1;
		}
		mutex->attr = new MutexAttributes;
		mutex->attr->placeholder = NULL;
		if (attr != NULL)
		{
			mutex->attr->placeholder = attr->placeholder;
		}
		return 0;
	}
		
	extern int mutex_destroy(Mutex *mutex)
	{
		delete mutex->attr;
		CloseHandle(mutex->handle);
	}

	extern int mutex_lock(Mutex *mutex)
	{
		if (WaitForSingleObject(mutex->handle, INFINITE) != WAIT_OBJECT_0)
		{
			return 1;
		}
		return 0;
	}

	extern int mutex_unlock(Mutex *mutex)
	{
		MutexRelease(mutex->handle);
		return 0;
	}
		
	extern int cond_init(Cond *cond, const CondAttributes *attr)
	{
		cond->attr = new CondAttributes;
		cond->attr->placeholder = NULL;
		if (attr != NULL)
		{
			cond->attr->placeholder = attr->placeholder;
		}
		cond->external_mutex = NULL;
		cond->internal_mutex = new Mutex;
		if (mutex_init(cond->internal_mutex) != 0)
		{
			return 1;
		}
		cond->wait_queue = std::queue<Thread>();
		cond->threads = std::unordered_map<Thread>();
		return 0;
	}
		
	extern int cond_destroy(Cond *cond)
	{
		mutex_lock(cond->internal_mutex);
		while (!cond->wait_queue.empty())
		{
			Thread tid = cond->wait_queue.front();
			cond->wait_queue.pop();
			if (__info.find(tid) != __info.end() && cond->threads.count(tid) == 1)
			{
				__info[tid]->waiting_on_cond = false;
			}
			cond->threads.erase(tid);
		}
		mutex_unlock(cond->internal_mutex);
		mutex_destory(cond->internal_mutex);
		delete cond->internal_mutex;
		delete cond->attr;
		return 0;
	}
		
	extern int cond_wait(Cond *cond, Mutex *mutex)
	{
		if (cond->external_mutex != NULL)
		{
			if (cond->external_mutex != mutex)
			{
				return 1;
			}
		}
		cond->external_mutex = mutex;
		Thread tid = GetCurrentThreadId();

		mutex_lock(cond->internal_mutex);
		__info[tid]->waiting_on_cond = true;
		cond->threads.insert(tid);
		cond->wait_queue.push(tid);
		mutex_unlock(cond->internal_mutex);

		if (!mutex_unlock(cond->mutex))
		{
			return 1;
		}
		// set up done, wait for cond now
		while (__info[tid]->waiting_on_cond)
		{
			Sleep(5);
		}
		// we aren't waiting, so return
		if (!mutex_lock(cond->external_mutex))
		{
			return 1;
		}
		return 0;
	}
		
	extern int cond_broadcast(Cond *cond);
	{
		while (!cond->wait_queue.empty())
		{
			int status = cond_signal(cond);
			if (status != 0)
			{
				return status;
			}
		}
		return 0;
	}
		
	extern int cond_signal(Cond *cond)
	{
		mutex_lock(cond->internal_mutex);
		Thread tid = 0;
		do {
			if (cond->wait_queue.empty())
			{
				mutex_unlock(cond->internal_mutex);
				return 1;
			}
			tid = cond->wait_queue.front();
			cond->wait_queue.pop();
		} while (cond->threads.count(tid) != 1);
			
		cond->threads.erase(tid);
		__info[tid]->waiting_on_cond = false;
		mutex_unlock(cond->internal_mutex);
	}
		
	static DWORD __run(void *arg)
	{
		RunnerArguments *ra = (RunnerArguments *) arg;

		// wait for the create function to finish creating us before continuing
		if (WaitForSingleObject(ra->start_mutex, INFINITE) != WAIT_OBJECT_0)
		{
			delete ra;
			__destroy_info(GetCurrentThreadId());
			return;
		}
		ReleaseMutex(ra->start_mutex);
		CloseHandle(ra->start_mutex);

		// pull out the args we need, drop them on the stack and free the passed in arg
		void *(*start_routine)(void *) = ra->start_routine;
		void *start_routine_arg = ra->start_routine_arg;
		delete ra;
		
		// execute the actual requested function
		void *ret_val = start_routine(start_routine_arg);

		// if someone could join on us, save the return value. Otherwise clean up immediately
		if (__info[GetCurrentThreadId()]->joinable)
		{
			__ret_values[GetCurrentThreadId()] = ret_val;
		}
		else
		{
			__destroy_info(GetCurrentThreadId());
		}
		return 0;
	}

	static void __create_info(Thread thread, HANDLE thread_handle, bool joinable)
	{
		__info[thread] = new Info;
		__info[thread]->name = new char[16];
		__info[thread]->name[0] = '\0';
		__info[thread]->handle = thread_handle;
		__info[thread]->joinable = joinable;
		__info[thread]->waiting_on_cond = false;
	}

	static void __destory_info(Thread thread)
	{
		Info *ti = __info[thread];
		// this could be called from __run on its own handle
		// TODO: is this okay?
		CloseHandle(ti->thread_handle);
		delete[] ti->name;
		delete ti;
		__info.erase(thread);
	}

} }
