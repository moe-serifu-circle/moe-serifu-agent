#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "event_handler.hpp"

namespace msa { namespace core {

	typedef enum status_type { CREATED, RUNNING, STOPPED } Status;

	typedef struct environment_type* Handle;

	extern int init(Handle *msa);
	extern int quit(Handle msa);
	extern int dispose(Handle msa);
	extern void subscribe(Handle msa, msa::event::Topic, msa::event::EventHandler);
	extern void unsubscribe(Handle msa, msa::event::Topic, msa::event::EventHandler);
	extern Status status(Handle msa);
	extern void push_event(Handle msa, msa::event::Event *e);
} }

#endif
