#ifndef MSA_EVENT_EVENT_HPP
#define MSA_EVENT_EVENT_HPP

#include <ctime>
#include <cstdint>

namespace msa { namespace event {

	typedef enum topic_type {
		#define MSA_EVENT_TOPIC(enum_name, priority)		enum_name,
		#include "event/topics.hpp"
		#undef MSA_EVENT_TOPIC
	} Topic;

	struct topic_attr;

	typedef struct event_type
	{
		Topic topic;
		const topic_attr *attributes;
		time_t generation_time;
		void *args;
	} Event;

	extern bool operator<(const Event &e1, const Event &e2);
	extern bool operator>(const Event &e1, const Event &e2);
	extern bool operator<=(const Event &e1, const Event &e2);
	extern bool operator>=(const Event &e1, const Event &e2);
	extern bool operator==(const Event &e1, const Event &e2);
	extern bool operator!=(const Event &e1, const Event &e2);

	extern const Event *create(Topic topic, void *args);
	extern void dispose(const Event *e);
	extern uint8_t get_priority(const Event *e);
	extern int max_topic_index();

} }

#endif
