#include "control.hpp"
#include "event.hpp"

#include <pthread.h>
#include <cstdio>
#include <queue>
#include <stack>
#include <map>

namespace msa { namespace core {
	
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

	struct event_dispatch_context_type {
		pthread_t edt;
		pthread_mutex_t queue_mutex;
		HandlerContext *current_handler;
		std::priority_queue<const msa::event::Event *> queue;
		std::map<msa::event::Topic, msa::event::EventHandler> handlers;
		std::stack<HandlerContext *> interrupted;
	};

	static int create_event_dispatch_context(EventDispatchContext **event);
	static int dispose_event_dispatch_context(EventDispatchContext *event);
	static void *event_start(void *args);

	static const msa::event::Event *peek_event(Handle msa);
	static const msa::event::Event *pop_event(Handle msa);

	static void *edt_start(void *args);
	static void edt_run(Handle hdl);
	static void edt_cleanup(Handle hdl);
	static const msa::event::Event *edt_poll_event_queue(Handle hdl);
	static void edt_interrupt_handler(Handle hdl);
	static void edt_spawn_handler(Handle hdl, const msa::event::Event *e);
	static void edt_dispatch_event(Handle hdl, const msa::event::Event *e);
	static void dispose_handler_context(HandlerContext *ctx);

	extern int init(Handle *msa)
	{
		environment_type *hdl = new environment_type;
		hdl->status = Status::CREATED;
		int create_status = create_event_dispatch_context(&hdl->event);
		if (create_status != 0)
		{
			return create_status;
		}
		create_status = pthread_create(&hdl->event->edt, NULL, edt_start, hdl);
		if (create_status != 0)
		{
			pthread_mutex_destroy(&hdl->event->queue_mutex);
			return create_status;
        	}
		*msa = hdl;
		return 0;
	}

	extern int quit(Handle msa)
	{
		msa->status = Status::STOP_REQUESTED;
		return 0;
	}

	extern int dispose(Handle msa)
	{
		if (msa->event != NULL)
		{
			dispose_event_dispatch_context(msa->event);
			msa->event = NULL;
		}
		delete msa;
		return 0;
	}

	extern void subscribe(Handle msa, msa::event::Topic t, msa::event::EventHandler handler)
	{
		msa->event->handlers[t] = handler;
	}

	extern void unsubscribe(Handle msa, msa::event::Topic t, msa::event::EventHandler handler)
	{
		msa->event->handlers[t] = NULL;
	}

	static int create_event_dispatch_context(EventDispatchContext **event)
	{
		EventDispatchContext *edc = new EventDispatchContext;
		edc->queue_mutex = PTHREAD_MUTEX_INITIALIZER;
		edc->current_handler = NULL;
		*event = edc;
		return 0;
	}

	static int dispose_event_dispatch_context(EventDispatchContext *event)
	{
		delete event;
		return 0;
	}

	static void *edt_start(void *args)
	{
		int retval; // include for portability according to man page
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &retval);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &retval);
		Handle hdl = (Handle) args;
		hdl->status = Status::RUNNING;
		while (hdl->status != Status::STOP_REQUESTED)
		{
			edt_run(hdl);
		}
		edt_cleanup(hdl);
	}

	static void edt_cleanup(Handle hdl)
	{
		if (hdl->event->current_handler != NULL)
		{
			dispose_handler_context(hdl->event->current_handler);
		}
		while (!hdl->event->interrupted.empty())
		{
			HandlerContext *ctx = hdl->event->interrupted.top();
			hdl->event->interrupted.pop();
			dispose_handler_context(ctx);
		}
		pthread_mutex_destroy(&hdl->event->queue_mutex);
		while (!hdl->event->queue.empty())
		{
			const msa::event::Event *e = hdl->event->queue.top();
			hdl->event->queue.pop();
			delete e;
		}
		hdl->status = Status::STOPPED;
	}


	static void edt_run(Handle hdl) {
		// check event_queue, decide if we want the current top
		const msa::event::Event *e = edt_poll_event_queue(hdl);
		if (e != NULL)
		{
			edt_dispatch_event(hdl, e);
		}
		
		EventDispatchContext *edc = hdl->event;
		// check if current task has finished
		if (edc->current_handler != NULL && !(edc->current_handler->running))
		{
			dispose_handler_context(edc->current_handler);
			edc->current_handler = NULL;
		}
		// if current task is clear, load up the next one that has been interrupted
		if (edc->current_handler == NULL && !edc->interrupted.empty())
		{
			HandlerContext *ctx = edc->interrupted.top();
			edc->interrupted.pop();
			msa::event::resume_handler(ctx->sync);
			edc->current_handler = ctx;
		}
		// TODO: Add a synthetic event for when queue has emptied and no events
	}

	static const msa::event::Event *edt_poll_event_queue(Handle hdl)
	{
		const msa::event::Event *e = NULL;
		pthread_mutex_lock(&hdl->event->queue_mutex);
		if (!hdl->event->queue.empty())
		{
			e = hdl->event->queue.top();
			// if we have a current event, check to see if we should replace it
			// with the event on the queue
			if (hdl->event->current_handler != NULL)
			{
				HandlerContext *ctx = hdl->event->current_handler;
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
			hdl->event->queue.pop();
		}
		pthread_mutex_unlock(&hdl->event->queue_mutex);
		return e;
	}

	static void edt_interrupt_handler(Handle hdl)
	{
		HandlerContext *ctx = hdl->event->current_handler;
		msa::event::suspend_handler(ctx->sync);
		// wait till it stops
		while (!msa::event::handler_suspended(ctx->sync)) {
			// we just wait here until we can go
			sleep_milli(10);
			// TODO: Force handler to stop if it takes too long
		}
		// okay now put it on the stack
		hdl->event->interrupted.push(ctx);
		// and clear the current
		hdl->event->current_handler = NULL;
	}

	static void edt_spawn_handler(Handle hdl, const msa::event::Event *e)
	{
		HandlerContext *new_ctx = new HandlerContext;
		new_ctx->event = e;
		new_ctx->handler_func = hdl->event->handlers[e->topic];
		msa::event::create_handler_sync(&new_ctx->sync);
		hdl->event->current_handler = new_ctx;
		new_ctx->running = (pthread_create(&new_ctx->thread, NULL, event_start, hdl) == 0);
	}

	static void edt_dispatch_event(Handle hdl, const msa::event::Event *e)
	{
		if (hdl->event->current_handler != NULL)
		{
			edt_interrupt_handler(hdl);
		}
		// start the thread (if we have a handler)
		if (hdl->event->handlers[e->topic] != NULL)
		{
			edt_spawn_handler(hdl, e);
		}
		else
		{
			delete e;
		}
	}

	static void dispose_handler_context(HandlerContext *ctx)
	{
		if (ctx->running)
		{
			if (msa::event::handler_suspended(ctx->sync))
			{
				msa::event::resume_handler(ctx->sync);
			}
			// let current event run through
			int err = pthread_join(ctx->thread, NULL);
		}
		// delete event
		msa::event::dispose(ctx->event);
		// delete sync handler
		msa::event::dispose_handler_sync(ctx->sync);
		delete ctx;
	}

	void *event_start(void *args)
	{
		Handle hdl = (Handle) args;
		HandlerContext *ctx = hdl->event->current_handler;
		ctx->handler_func(ctx->event, ctx->sync);
		ctx->running = false;
	}

	extern void push_event(Handle msa, msa::event::Event *e)
	{
		e->env = msa;
		pthread_mutex_lock(&msa->event->queue_mutex);
		msa->event->queue.push(e);
		pthread_mutex_unlock(&msa->event->queue_mutex);
	}

	static const msa::event::Event *peek_event(Handle msa)
	{
		pthread_mutex_lock(&msa->event->queue_mutex);
		const msa::event::Event *e = msa->event->queue.top();
		pthread_mutex_unlock(&msa->event->queue_mutex);
		return e;
	}

	static const msa::event::Event *pop_event(Handle msa)
	{
		pthread_mutex_lock(&msa->event->queue_mutex);
		const msa::event::Event *e = msa->event->queue.top();
		msa->event->queue.pop();
		pthread_mutex_unlock(&msa->event->queue_mutex);
	}

} }
