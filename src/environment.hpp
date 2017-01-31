#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

namespace msa { namespace io {

	typedef struct input_context_type InputContext;

} }

namespace msa { namespace core {

	typedef enum status_type { CREATED, RUNNING, STOP_REQUESTED, STOPPED } Status;
	
	typedef struct event_dispatch_context_type EventDispatchContext;

	struct environment_type
	{
		Status status;
		EventDispatchContext *event;
		msa::io::InputContext *input;
	};

	typedef struct environment_type* Handle;

} }


#endif
