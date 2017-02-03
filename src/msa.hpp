#ifndef MSA_HPP
#define MSA_HPP

// DO NOT INCLUDE ANY MSA HEADER FILES HERE!
// If you find you have to, re-think your design.
// It will break here.

namespace msa {

	namespace io {

		typedef struct input_context_type InputContext;

	}

	namespace event {
		
		typedef struct event_dispatch_context_type EventDispatchContext;
		
	}

	namespace agent {

		typedef struct agent_context_type AgentContext;

	}

	typedef enum status_type { CREATED, RUNNING, STOP_REQUESTED, STOPPED } Status;

	struct environment_type
	{
		Status status;
		msa::event::EventDispatchContext *event;
		msa::io::InputContext *input;
		msa::agent::AgentContext *agent;
	};

	typedef struct environment_type* Handle;

	extern int init(Handle *hdl);
	extern int quit(Handle hdl);
	extern int dispose(Handle hdl);

}

#endif
