#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include "event.hpp"

namespace msa { namespace event {

	typedef struct handler_synchronization_type HandlerSync;

	typedef void (*EventHandler)(const Event *const e, HandlerSync *const sync);

	// creates a handler sync and initializes the variables in it
	extern void create_handler_sync(HandlerSync **sync);

	// destroys handler sync and frees resources associated with it
	extern void dispose_handler_sync(HandlerSync *sync);

	// called on a handler to tell it to suspend at next safe point
	extern void suspend_handler(HandlerSync *sync);

	// called on a handler to tell it to 
	extern void resume_handler(HandlerSync *sync);

	// check if a handler has entered an interrupt point and is waiting
	// to be resumed.
	extern bool handler_suspended(HandlerSync *sync);

	// Defines a safe point inside of an event handler. Use these
	// to mark where a handler can safely be suspended/interrupted
	extern void HANDLER_INTERRUPT_POINT(HandlerSync *sync);

} }

#endif
