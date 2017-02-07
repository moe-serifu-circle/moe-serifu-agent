#ifndef MSA_HPP
#define MSA_HPP

// DO NOT INCLUDE ANY MSA HEADER FILES HERE!
// If you find you have to, re-think your design.
// It will break here.

#define MSA_SUCCESS 0
#define MSA_ERR_EVENT 1
#define MSA_ERR_INPUT 2
#define MSA_ERR_AGENT 3
#define MSA_ERR_CONFIG 4
#define MSA_ERR_CMD 5

namespace msa {

	namespace input {

		typedef struct input_context_type InputContext;

	}

	namespace event {
		
		typedef struct event_dispatch_context_type EventDispatchContext;
		
	}

	namespace agent {

		typedef struct agent_context_type AgentContext;

	}

	namespace cmd {

		typedef struct command_context_type CommandContext;

	}

	namespace log {

		typedef struct log_context_type LogContext;

	}

	typedef enum status_type { CREATED, RUNNING, STOP_REQUESTED, STOPPED } Status;

	struct environment_type
	{
		Status status;
		msa::event::EventDispatchContext *event;
		msa::input::InputContext *input;
		msa::agent::AgentContext *agent;
		msa::cmd::CommandContext *cmd;
	};

	typedef struct environment_type* Handle;

	extern int init(Handle *hdl, const char *config_path);
	extern int quit(Handle hdl);
	extern int dispose(Handle hdl);

}

#endif
