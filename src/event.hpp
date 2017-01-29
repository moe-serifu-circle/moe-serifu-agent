#ifndef EVENT_HPP
#define EVENT_HPP

#include <ctime>
#include <cstdint>

namespace msa { namespace event {

	// MUST BE SEQUENTIAL!! Do not assign specific constants
	typedef enum topic_type {
		EVENT_STACK_CLEARED,
		EVENT_HANDLED,
		EVENT_INTERRUPTED,
		COMMAND_ANNOUNCE
	} Topic;

	struct topic_attr;

	typedef struct event_type
	{
		Topic topic;
		const topic_attr *attributes;
		time_t generation_time;
		void *args;
	} Event;

	inline bool operator<(const Event &e1, const Event &e2);
	inline bool operator>(const Event &e1, const Event &e2);
	inline bool operator<=(const Event &e1, const Event &e2);
	inline bool operator>=(const Event &e1, const Event &e2);
	inline bool operator==(const Event &e1, const Event &e2);
	inline bool operator!=(const Event &e1, const Event &e2);

	extern const Event *create(Topic topic, void *args);
	extern void dispose(Event *e);
	extern uint8_t get_priority(Event *e);
	extern int max_topic_index();

} }

#endif
