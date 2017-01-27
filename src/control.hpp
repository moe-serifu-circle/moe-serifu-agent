#ifndef CONTROL_HPP
#define CONTROL_HPP

namespace msa::control {

	typedef enum status_type { CREATED, RUNNING, STOPPED } Status;

	typedef struct environment_type* Handle;

	extern int init(Handle *msa);
	extern int quit(Handle msa);
	extern Status status(Handle msa);
}

#endif
