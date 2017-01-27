#include "control.hpp"

#include <pthread.h>
#include <cstdio>
#include <unistd.h>

namespace msa::control {

	static void *msa_main(void *args);

	struct environment_type
	{
		pthread_t exec_thread;
		Status status;
	};

	extern int init(Handle *msa)
	{
		environment_type *hdl = new environment_type;
		hdl->status = CREATED;
		int create_status = pthread_create(&hdl->exec_thread, NULL, msa_main, hdl);
		if (create_status != 0)
		{
			delete hdl;
			return create_status;
        	}
		*msa = hdl;
		return 0;
	}

	extern int quit(Handle msa)
	{
		int err = pthread_cancel(msa->exec_thread);
		if (err != 0)
		{
			return err;
		}
		err = pthread_join(msa->exec_thread, NULL);
		if (err != 0)
		{
			return err;
		}
		delete msa;
		return 0;
	}

	extern Status status(Handle msa)
	{
		return msa->status;
	}

	static void *msa_main(void *args)
	{
		int retval; // include for portability according to man page
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &retval);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &retval);
		while (true)
		{
			printf("Cute Anime Girl: \"I am alive\"!\n");
			sleep(2);
		}
	}
}
