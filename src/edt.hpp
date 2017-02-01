#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "environment.hpp"
#include "event_handler.hpp"

namespace msa { namespace event {

	extern int init_edt(msa::Handle *msa);
	extern int quit_edt(msa::Handle msa);
	// TODO: drop dispose of handle in msa:: namespace
	extern int dispose(msa::Handle msa);
	extern void subscribe(msa::Handle msa, Topic, EventHandler);
	extern void unsubscribe(msa::Handle msa, Topic, EventHandler);
	extern void push(msa::Handle msa, const Event *e);
} }

#endif
