#ifndef MSA_CORE_HPP
#define MSA_CORE_HPP

namespace msa_core {

	typedef enum status_t { CREATED, RUNNING, STOPPED } STATUS;

	typedef struct env_t* HANDLE;

	extern int init(HANDLE *msa));
	extern int quit(HANDLE msa);
	extern STATUS status(HANDLE msa);
}

#endif
