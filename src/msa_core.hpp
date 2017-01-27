#ifndef MSA_CORE_H
#define MSA_CORE_H

namespace msa_core {

	typedef enum status_t { CREATED, RUNNING, STOPPED } STATUS;

	typedef struct env_t* HANDLE;

	extern int init(HANDLE *msa));
	extern int quit(HANDLE msa);
	extern STATUS status(HANDLE msa);
}

#endif
