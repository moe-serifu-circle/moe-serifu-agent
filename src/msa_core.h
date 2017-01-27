#ifndef MSA_CORE_H
#define MSA_CORE_H

namespace msa_core {

	enum status_t { CREATED, RUNNING, STOPPED };

	typedef struct sys_handle_t sys_handle;

	sys_handle *init();
	void stop(sys_handle *handle);
	status_t status(sys_handle *handle);
}

#endif
