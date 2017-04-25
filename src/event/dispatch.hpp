#ifndef MSA_EVENT_DISPATCH_HPP
#define MSA_EVENT_DISPATCH_HPP

#include "msa.hpp"
#include "event/handler.hpp"
#include "cfg/cfg.hpp"

namespace msa { namespace event {

	extern int init(msa::Handle msa, const msa::cfg::Section &config);
	extern int quit(msa::Handle msa);
	extern void subscribe(msa::Handle msa, Topic, EventHandler);
	extern void unsubscribe(msa::Handle msa, Topic, EventHandler);
	extern void generate(msa::Handle msa, const Topic, void *args);
} }

#endif
