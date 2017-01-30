#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

namespace msa { namespace core {

	typedef enum status_type { CREATED, RUNNING, STOP_REQUESTED, STOPPED } Status;
	
	typedef struct event_dispatch_context_type EventDispatchContext;

	struct environment_type
	{
		Status status;
		EventDispatchContext *event;
	};

	typedef struct environment_type* Handle;

} }


#endif
