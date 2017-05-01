#ifndef MSA_EVENT_EVENT_HPP
#define MSA_EVENT_EVENT_HPP

#include <ctime>
#include <cstdint>
#include <string>

namespace msa { namespace event {

	enum class Topic
	{
		#define MSA_EVENT_TOPIC(enum_name, priority)		enum_name,
		#include "event/topics.hpp"
		#undef MSA_EVENT_TOPIC
	};

	// wraps the actual event args so that users of the event API do not have to manually provide
	// move, copy, and delete operators for every actual type of event args unless they wish to
	// implement IArgs
	class IArgs {
		public:
			virtual ~IArgs() {}
			virtual IArgs *copy() const = 0;
	};
	
	// concrete implementation of IArgs that just defers copy operation to resident object
	template<typename T>
	class Args : public IArgs
	{
		public:
			Args(const T &wrapped) : args(new T(wrapped))
			{}

			virtual ~Args()
			{
				delete args;
			}

			Args(const Args<T> &other) : args(new T(*other.args))
			{}

			Args &operator=(const Args<T> &other)
			{
				T *temp_args = new T(*other.args);
				delete args;
				args = temp_args;
				return *this;
			}

			virtual IArgs *copy() const
			{
				return new Args<T>(*this);
			}

			const T &get_args() const
			{
				return *args;
			}

		private:
			T *args;
	};
	
	template<typename T>
	Args<T> wrap(const T &arg)
	{
		return Args<T>(arg);
	}

	struct topic_attr;

	typedef struct event_type
	{
		Topic topic;
		const topic_attr *attributes;
		time_t generation_time;
		IArgs *args;
	} Event;

	extern bool operator<(const Event &e1, const Event &e2);
	extern bool operator>(const Event &e1, const Event &e2);
	extern bool operator<=(const Event &e1, const Event &e2);
	extern bool operator>=(const Event &e1, const Event &e2);
	extern bool operator==(const Event &e1, const Event &e2);
	extern bool operator!=(const Event &e1, const Event &e2);
	
	extern const Event *create(Topic topic, const IArgs &args);
	extern void dispose(const Event *e);
	extern uint8_t get_priority(const Event *e);
	extern int max_topic_index();
	extern std::string topic_str(Topic t);

} }

#endif
