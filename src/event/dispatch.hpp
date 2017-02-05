#ifndef EDT_HPP
#define EDT_HPP

#include "msa.hpp"
#include "event/handler.hpp"
#include "configuration.hpp"

namespace msa { namespace event {

	extern int init(msa::Handle msa, const ConfigSection &config);
	extern int quit(msa::Handle msa);
	extern void subscribe(msa::Handle msa, Topic, EventHandler);
	extern void unsubscribe(msa::Handle msa, Topic, EventHandler);
	extern void generate(msa::Handle msa, const Topic, void *args);
} }

#endif
