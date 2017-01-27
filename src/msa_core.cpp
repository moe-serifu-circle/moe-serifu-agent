#include "msa_core.h"

#include <pthread.h>
#include <stdlib.h>

namespace msa_core {

	struct sys_handle_t
	{
		status_t status;
	};

	sys_handle *init()
	{
		sys_handle *hdl = new sys_handle;
		hdl->status = CREATED;
		
	}

	

}
