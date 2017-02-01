#ifndef EDT_HPP
#define EDT_HPP

#include "environment.hpp"
#include "event_handler.hpp"

namespace msa { namespace event {

	extern int init(msa::Handle *msa);
	extern int quit(msa::Handle msa);
	// TODO: drop dispose of handle in msa:: namespace
	extern int dispose(msa::Handle msa);
	extern void subscribe(msa::Handle msa, Topic, EventHandler);
	extern void unsubscribe(msa::Handle msa, Topic, EventHandler);
	extern void push(msa::Handle msa, const Event *e);
} }

#endif
