#ifndef EVENT_HPP
#define EVENT_HPP

#include <ctime>

namespace msa::event {

	// MUST BE SEQUENTIAL!! Do not assign specific constants
	typedef enum topic_type {
		EVENT_STACK_CLEARED,
		EVENT_HANDLED,
		EVENT_INTERRUPTED
	} Topic;

	extern struct topic_attr;

	typedef struct event_type Event
	{
		Topic topic;
		const topic_attr &attributes;
		time_t generation_time;
		void *args;
	};

	extern const Event *create(Topic topic, void *args);
	extern void dispose(Event *e);

}

#endif
