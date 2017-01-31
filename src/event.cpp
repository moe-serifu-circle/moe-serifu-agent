#include "event.hpp"

#include <stdexcept>
#include <unistd.h>

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
		{10},
		// COMMAND_ANNOUNCE
		{2},
		// COMMAND_EXIT
		{1}
	};

	inline bool operator<(const Event &e1, const Event &e2)
	{
		return e1.attributes->priority < e2.attributes->priority;
	}

	inline bool operator>(const Event &e1, const Event &e2)
	{
		return (e2 < e1);
	}

	inline bool operator<=(const Event &e1, const Event &e2)
	{
		return !(e1 > e2);
	}

	inline bool operator>=(const Event &e1, const Event &e2)
	{
		return !(e1 < e2);
	}

	inline bool operator==(const Event &e1, const Event &e2)
	{
		return e1.attributes->priority == e2.attributes->priority;
	}

	inline bool operator!=(const Event &e1, const Event &e2)
	{
		return !(e1 == e2);
	}

	extern int max_topic_index()
	{
		return sizeof(topic_attr_table) / sizeof(struct topic_attr) - 1;
	}

	static const struct topic_attr *get_topic_attr(Topic t)
	{
		if (t > max_topic_index() || t < 0)
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

	extern void dispose(const Event *e)
	{
		delete e;
	}

	extern uint8_t get_priority(const Event *e)
	{
		return e->attributes->priority;
	}

	#undef ATTR_INDEX_MAX
} }
