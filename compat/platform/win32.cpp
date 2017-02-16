
#include <map>

namespace msa { namespace platform {

	namespace thread {

		typedef struct attr_type {
			LPSECURITY_ATTRIBUTES sec,
			bool detach;
		} Attributes;
		
		typedef struct mutex_type
		{
			HANDLE handle;
		} Mutex;

		typedef struct mutex_attr_type
		{
			void *placeholder;
		} MutexAttributes;
		
		typedef struct thread_info_type
		{
			HANDLE handle;
			char *name;
			bool joinable;
		} ThreadInfo;

		typedef struct thread_runner_args
		{
			void *start_routine_arg;
			void *(*start_routine)(void *);
			HANDLE start_mutex;
		} ThreadRunnerArgs;
		
		static DWORD __run(void *);
		static void __create_info(Thread thread, HANDLE handle, bool joinable);
		static void __destroy_info(Thread thread);
		
		static std::map<Thread, ThreadInfo *> __info;
		static std::map<Thread, void *> __ret_values;

		extern int create(Thread *thread, const Attributes *attr, void *(*start_routine)(void *), void *arg)
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
			
			ThreadRunnerArgs *tra = new ThreadRunnerArgs;
			tra->arg = arg;
			tra->start_routine = start_routine;
			tra->start_mutex = start_mutex;
			
			HANDLE thread_handle = CreateThread(sec, 0, __run, tra, 0, thread);
			if (thread_handle == NULL)
			{
				delete tra;
				ReleaseMutex(start_mutex);
				CloseHandle(start_Mutex);
				return 1;
			}
			__create_info(*thread, thread_handle, joinable);
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
			__ret_values,erase(thread);
			__destroy_info(thread);
			return 0;
		}

		extern int set_name(Thread tid, const char *name)
		{
			if (__info.find(tid) == __info.end())
			{
				__info[tid] = new ThreadInfo;
				__info[tid]->name
			}
			__names[tid] = new char[16];
			strncpy(__names[tid], name, 15);
			__names[tid][15] = '\0';
		}

		extern int get_name(Thread tid, char *name, size_t len)
		{
			if (__names.find(tid) == __names.end())
			{
				set_name(tid, "(not set)");
			}
			strncpy(name, __names[tid], len);
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
			delete attr;
		}
		
		extern int mutex_init(Mutex *mutex, const MutexAttributes *attr)
		{
			mutex->handle = CreateMutex(NULL, false, NULL);
			if (mutex->handle == NULL)
			{
		 		return 1;
			}
			return 0;
		}
		
		extern int mutex_destoy(Mutex *mutex)
		{
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
		}
		
		static DWORD __run(void *arg)
		{
			ThreadRunnerArguments *tra = (ThreadRunnerArguments *) arg;

			// wait for the create function to finish creating us before continuing
			if (WaitForSingleObject(tra->start_mutex, INFINITE) != WAIT_OBJECT_0)
			{
				delete tra;
				__destroy_info(GetCurrentThreadId());
				return;
			}
			ReleaseMutex(tra->start_mutex);
			CloseHandle(tra->start_mutex);

			// pull out the args we need, drop them on the stack and free the passed in arg
			void *(*start_routine)(void *) = tra->start_routine;
			void *start_routine_arg = tra->start_routine_arg;
			delete tra;

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
		}

		static void __create_info(Thread thread, HANDLE thread_handle, bool joinable)
		{
			__info[thread] = new ThreadInfo;
			__info[thread]->name = new char[16];
			__info[thread]->handle = thread_handle;
			__info[thread]->joinable = joinable;
		}

		static void __destory_info(Thread thread)
		{
			ThreadInfo *ti = __info[thread];
			// this could be called from __run on its own handle
			// TODO: is this okay?
			CloseHandle(ti->thread_handle);
			delete[] ti->name;
			delete ti;
			__info.erase(thread);
		}
	}

} }
