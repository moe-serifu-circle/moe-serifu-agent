#include "event.hpp"

#include <stdexcept>

namespace msa { namespace event {

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

	#define ATTR_INDEX_MAX (sizeof(topic_attr_table) / sizeof(struct topic_attr) - 1)

	static const struct topic_attr *get_topic_attr(Topic t)
	{
		if (t > ATTR_INDEX_MAX || t < 0)
		{
			throw std::invalid_argument("unknown topic");
		}
		size_t idx = static_cast<size_t>(t);
		return (topic_attr_table + idx);
	}

	extern const Event *create(Topic topic, void *args)
	{
		Event *e = new Event;
		e->generation_time = time(NULL);
		e->attributes = get_topic_attr(topic);
		e->topic = topic;
		e->args = args;
		return e;
	}

	extern void dispose(Event *e)
	{
		delete e;
	}

	#undef ATTR_INDEX_MAX
} }
