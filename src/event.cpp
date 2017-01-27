#include "event.hpp"

namespace msa::event {

	struct topic_attr {
		uint8_t priority;
	};

	static const struct topic_attr topic_attr_table[] = {
		//EVENT_STACK_CLEARED
		{10},
		//EVENT_HANDLED
		{10},
		//EVENT_INTERRUPTED
		{10}
	};

	#define ATTR_INDEX_MAX (sizeof(topic_attr_table) / sizeof(struct topic_attr))

	


	#undef ATTR_INDEX_MAX
}
