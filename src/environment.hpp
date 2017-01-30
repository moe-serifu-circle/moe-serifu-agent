#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <pthread.h>

namespace msa { namespace core {

	typedef enum status_type { CREATED, RUNNING, STOPPED } Status;

	typedef struct environment_type* Handle;

	typedef struct handler_context_type HandlerContext;

	typedef struct event_context_type EventContext;

	struct environment_type
	{
		pthread_t edt; // event dispatch thread
		Status status;
		HandlerContext *current_handler;
		EventContext 
		pthread_mutex_t event_mutex;
		std::priority_queue<const msa::event::Event *> event_queue;
		std::map<msa::event::Topic, msa::event::EventHandler> handlers;
		std::stack<HandlerContext *> interrupted_handlers;
	};

} }


#endif
