#include "control.hpp"
#include "event.hpp"

#include <pthread.h>
#include <cstdio>
#include <queue>
#include <stack>
#include <map>

namespace msa { namespace control {

	#define SLEEP_NANO(x) do {struct timespec u__tSpec

	static void sleep_milli(int millisec) {
		struct timespec t;
		t.tv_nsec = (uint64_t) millisec * 1000000000;
		nanosleep(&t, NULL);
	}

	typedef struct handler_context_type {
		const msa::event::Event *event;
		msa::event::EventHandler handler_func;
		msa::event::HandlerSync *sync;
		bool running;
		pthread_t thread;
	} HandlerContext;

	struct environment_type
	{
		pthread_t edt; // event dispatch thread
		Status status;
		HandlerContext *current_handler;
		pthread_mutex_t event_mutex;
		std::priority_queue<const msa::event::Event *> event_queue;
		std::map<msa::event::Topic, msa::event::EventHandler> handlers;
		std::stack<HandlerContext *> interrupted_handlers;
	};

	static void *event_start(void *args);

	static const msa::event::Event *peek_event(Handle msa);
	static const msa::event::Event *pop_event(Handle msa);

	static void *edt_start(void *args);
	static void edt_run(Handle hdl);
	static const msa::event::Event *edt_poll_event_queue(Handle hdl);
	static void edt_interrupt_handler(Handle hdl);
	static void edt_spawn_handler(Handle hdl, const msa::event::Event *e);
	static void edt_dispatch_event(Handle hdl, const msa::event::Event *e);
	static void edt_dispose_handler_context(Handle hdl);

	extern int init(Handle *msa)
	{
		environment_type *hdl = new environment_type;
		hdl->status = Status::CREATED;
		hdl->event_mutex = PTHREAD_MUTEX_INITIALIZER;
		hdl->current_handler = NULL;
		int create_status = pthread_create(&hdl->edt, NULL, edt_start, hdl);
		if (create_status != 0)
		{
			pthread_mutex_destroy(&hdl->event_mutex);
			return create_status;
        	}
		*msa = hdl;
		return 0;
	}

	extern int quit(Handle msa)
	{
		int err = pthread_cancel(msa->edt);
		if (err != 0)
		{
			return err;
		}
		err = pthread_join(msa->edt, NULL);
		if (err != 0)
		{
			return err;
		}
		pthread_mutex_destroy(&msa->event_mutex);
		msa->status = Status::STOPPED;
		return 0;
	}

	extern Status status(Handle msa)
	{
		return msa->status;
	}

	extern int dispose(Handle msa)
	{
		delete msa;
		return 0;
	}

	extern void subscribe(Handle msa, msa::event::Topic t, msa::event::EventHandler handler)
	{
		msa->handlers[t] = handler;
	}

	extern void unsubscribe(Handle msa, msa::event::Topic t, msa::event::EventHandler handler)
	{
		msa->handlers[t] = NULL;
	}

	static void *edt_start(void *args)
	{
		int retval; // include for portability according to man page
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &retval);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &retval);
		Handle hdl = (Handle) args;
		hdl->status = Status::RUNNING;
		while (true)
		{
			edt_run(hdl);
		}
	}


	static void edt_run(Handle hdl) {
		// check event_queue, decide if we want the current top
		const msa::event::Event *e = edt_poll_event_queue(hdl);
		if (e != NULL)
		{
			edt_dispatch_event(hdl, e);
		}
		// check if current task has finished
		if (hdl->current_handler != NULL && !(hdl->current_handler->running))
		{
			edt_dispose_handler_context(hdl);
		}
		// if current task is clear, load up the next one that has been interrupted
		if (hdl->current_handler == NULL && !hdl->interrupted_handlers.empty())
		{
			HandlerContext *ctx = hdl->interrupted_handlers.top();
			hdl->interrupted_handlers.pop();
			msa::event::resume_handler(ctx->sync);
			hdl->current_handler = ctx;
		}
		// TODO: Add a synthetic event for when queue has emptied and no events
	}

	static const msa::event::Event *edt_poll_event_queue(Handle hdl)
	{
		const msa::event::Event *e = NULL;
		pthread_mutex_lock(&hdl->event_mutex);
		if (!hdl->event_queue.empty())
		{
			e = hdl->event_queue.top();
			// if we have a current event, check to see if we should replace it
			// with the event on the queue
			if (hdl->current_handler != NULL)
			{
				HandlerContext *ctx = hdl->current_handler;
				uint8_t cur_prior = msa::event::get_priority(ctx->event);
				uint8_t new_prior = msa::event::get_priority(e);
				if (cur_prior >= new_prior)
				{
					e = NULL;
				}
				// otherwise finish our current task before handling next event
			}
		}
		if (e != NULL)
		{
			hdl->event_queue.pop();
		}
		pthread_mutex_unlock(&hdl->event_mutex);
		return e;
	}

	static void edt_interrupt_handler(Handle hdl)
	{
		HandlerContext *ctx = hdl->current_handler;
		msa::event::suspend_handler(ctx->sync);
		// wait till it stops
		while (!msa::event::handler_suspended(ctx->sync)) {
			// we just wait here until we can go
			sleep_milli(10);
		}
		// okay now put it on the stack
		hdl->interrupted_handlers.push(ctx);
		// and clear the current
		hdl->current_handler = NULL;
	}

	static void edt_spawn_handler(Handle hdl, const msa::event::Event *e)
	{
		HandlerContext *new_ctx = new HandlerContext;
		new_ctx->event = e;
		new_ctx->handler_func = hdl->handlers[e->topic];
		msa::event::create_handler_sync(&new_ctx->sync);
		hdl->current_handler = new_ctx;
		new_ctx->running = (pthread_create(&new_ctx->thread, NULL, event_start, hdl) == 0);
	}

	static void edt_dispatch_event(Handle hdl, const msa::event::Event *e)
	{
		if (hdl->current_handler != NULL)
		{
			edt_interrupt_handler(hdl);
		}
		// start the thread (if we have a handler)
		if (hdl->handlers[e->topic] != NULL)
		{
			edt_spawn_handler(hdl, e);
		}
		else
		{
			delete e;
		}
	}

	static void edt_dispose_handler_context(Handle hdl)
	{
		// we can delete this guy, he's done
		HandlerContext *ctx = hdl->current_handler;
		// delete event
		msa::event::dispose(ctx->event);
		// delete sync handler
		msa::event::dispose_handler_sync(ctx->sync);
		delete ctx;
		hdl->current_handler = NULL;
	}

	void *event_start(void *args)
	{
		Handle hdl = (Handle) args;
		HandlerContext *ctx = hdl->current_handler;
		//ctx->event->args = hdl;
		ctx->handler_func(ctx->event, ctx->sync);
		ctx->running = false;
	}

	extern void push_event(Handle msa, const msa::event::Event *e)
	{
		pthread_mutex_lock(&msa->event_mutex);
		msa->event_queue.push(e);
		pthread_mutex_unlock(&msa->event_mutex);
	}

	static const msa::event::Event *peek_event(Handle msa)
	{
		pthread_mutex_lock(&msa->event_mutex);
		const msa::event::Event *e = msa->event_queue.top();
		pthread_mutex_unlock(&msa->event_mutex);
		return e;
	}

	static const msa::event::Event *pop_event(Handle msa)
	{
		pthread_mutex_lock(&msa->event_mutex);
		const msa::event::Event *e = msa->event_queue.top();
		msa->event_queue.pop();
		pthread_mutex_unlock(&msa->event_mutex);
	}

} }
