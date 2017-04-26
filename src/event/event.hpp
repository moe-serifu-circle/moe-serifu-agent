#ifndef MSA_EVENT_EVENT_HPP
#define MSA_EVENT_EVENT_HPP

#include <ctime>
#include <cstdint>
#include <string>

namespace msa { namespace event {

	typedef enum topic_type {
		#define MSA_EVENT_TOPIC(enum_name, priority)		enum_name,
		#include "event/topics.hpp"
		#undef MSA_EVENT_TOPIC
	} Topic;

	// wraps the actual event args so that users of the event API do not have to manually provide
	// move, copy, and delete operators for every actual type of event args unless they wish to
	// implement IEventArgs
	class IEventArgs {
		public:
			virtual IEventArgs *copy() = 0;
	};
	
	// concrete implementation of EventArgs that just differs copy operation to resident object
	template<typename T>
	class EventArgs : public IEventArgs
	{
		public:
			EventArgs(const T &wrapped) : args(new T(wrapped))
			{}

			~EventArgs()
			{
				delete args;
			}

			EventArgs(const EventArgs<T> &other)
			{
				args = new T(other.args);
			}

			EventArgs &operator=(const EventArgs<T> &other)
			{
				T *temp_args = new T(other.args);
				delete args;
				args = temp_args;
				return *this;
			}

			virtual IEventArgs *copy()
			{
				return new EventArgs<T>(*this);
			}

			const T &get_args()
			{
				return *args;
			}

		private:
			T *args;
	};

	struct topic_attr;

	typedef struct event_type
	{
		Topic topic;
		const topic_attr *attributes;
		time_t generation_time;
		IEventArgs *args;
	} Event;

	extern bool operator<(const Event &e1, const Event &e2);
	extern bool operator>(const Event &e1, const Event &e2);
	extern bool operator<=(const Event &e1, const Event &e2);
	extern bool operator>=(const Event &e1, const Event &e2);
	extern bool operator==(const Event &e1, const Event &e2);
	extern bool operator!=(const Event &e1, const Event &e2);

	template<class T>
	extern const Event *create(Topic topic, const T &args)
	{
		Event *e = new Event;
		e->generation_time = time(NULL);
		e->attributes = get_topic_attr(topic);
		e->topic = topic;
		e->args = new EventArgs<T>(args);
		return e;
	}
	
	extern void dispose(const Event *e);
	extern uint8_t get_priority(const Event *e);
	extern int max_topic_index();
	extern std::string topic_str(Topic t);

} }

#endif
