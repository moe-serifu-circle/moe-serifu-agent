#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "environment.hpp"
#include "event_handler.hpp"

namespace msa { namespace core {

	extern int init(Handle *msa);
	extern int quit(Handle msa);
	extern int dispose(Handle msa);
	extern void subscribe(Handle msa, msa::event::Topic, msa::event::EventHandler);
	extern void unsubscribe(Handle msa, msa::event::Topic, msa::event::EventHandler);
	extern void push_event(Handle msa, const msa::event::Event *e);
} }

#endif
