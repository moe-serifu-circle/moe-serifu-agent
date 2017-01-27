#include "msa_core.hpp"

#include <pthread.h>
#include <stdio.h>

namespace msa_core {
    
    static void *agent_main(void *args);

	struct env_t
	{
        pthread_t exec_thread;
		STATUS status;
	};    

	extern int init(HANDLE *msa)
	{
		ent_t *hdl = new env_t;
		hdl->status = CREATED;
        int t_status;
		create_status = pthread_create(&hdl->exec_thread, NULL, agent_main, hdl);
        if (created_status != 0) {
            delete hdl;
            return created_status;
        }
        *msa = hdl;
        return 0;
	}
    
    extern int quit(HANDLE msa)
    {
        int err = pthread_cancel(msa->exec_thread);
        if (err != 0) {
            return err;
        }
        err = pthread_join(msa->exec_thread, NULL);
        if (err != 0) {
            return err;
        }
        delete msa;
        return 0;
    }
    
    extern STATUS status(HANDLE msa)
    {
        return msa->status;
    }
    
    static void *agent_main(void *args)
    {
        int retval; // include for portability according to man page
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &retval);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &retval);
        while (true)
        {
            printf("I am alive!\n");
        }
    }
    
    
	

}
