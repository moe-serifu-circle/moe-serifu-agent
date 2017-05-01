#include "event/event.hpp"

#include <stdexcept>
#include <cstdint>

namespace msa { namespace event {

	struct topic_attr {
		uint8_t priority;
	};

	static const struct topic_attr topic_attr_table[] = {
		#define MSA_EVENT_TOPIC(enum_name, priority)		{priority},
		#include "event/topics.hpp"
		#undef MSA_EVENT_TOPIC
	};

	extern bool operator<(const Event &e1, const Event &e2)
	{
		return e1.attributes->priority < e2.attributes->priority;
	}

	extern bool operator>(const Event &e1, const Event &e2)
	{
		return (e2 < e1);
	}

	extern bool operator<=(const Event &e1, const Event &e2)
	{
		return !(e1 > e2);
	}

	extern bool operator>=(const Event &e1, const Event &e2)
	{
		return !(e1 < e2);
	}

	extern bool operator==(const Event &e1, const Event &e2)
	{
		return e1.attributes->priority == e2.attributes->priority;
	}

	extern bool operator!=(const Event &e1, const Event &e2)
	{
		return !(e1 == e2);
	}

	extern int max_topic_index()
	{
		return sizeof(topic_attr_table) / sizeof(struct topic_attr) - 1;
	}

	static const struct topic_attr *get_topic_attr(Topic t)
	{
		int t_val = static_cast<int>(t);
		if (t_val > max_topic_index() || t_val < 0)
		{
			throw std::invalid_argument("unknown topic; did you add the topic to the attr table?");
		}
		size_t idx = static_cast<size_t>(t);
		return (topic_attr_table + idx);
	}
	
	extern const Event *create(Topic topic, const IArgs &args)
	{
		Event *e = new Event;
		e->generation_time = time(NULL);
		e->attributes = get_topic_attr(topic);
		e->topic = topic;
		e->args = args.copy();
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
	
	extern std::string topic_str(Topic t)
	{
		#define MSA_EVENT_TOPIC(enum_name, priority)		if (t == Topic::enum_name) return #enum_name;
		#include "event/topics.hpp"
		#undef MSA_EVENT_TOPIC
		return std::to_string(static_cast<int>(t));
	}
} }
