#ifndef MSA_HPP
#define MSA_HPP

namespace msa {

	namespace io {

		typedef struct input_context_type InputContext;

	}

	namespace event {
		
		typedef struct event_dispatch_context_type EventDispatchContext;
		
	}

	typedef enum status_type { CREATED, RUNNING, STOP_REQUESTED, STOPPED } Status;

	struct environment_type
	{
		Status status;
		msa::event::EventDispatchContext *event;
		msa::io::InputContext *input;
	};

	typedef struct environment_type* Handle;

	extern int init(Handle *hdl);
	extern int quit(Handle hdl);
	extern int dispose(Handle hdl);

}

#endif
