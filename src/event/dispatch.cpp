#include "event/dispatch.hpp"
#include "util.hpp"

#include <pthread.h>
#include <cstdio>
#include <queue>
#include <stack>
#include <map>

namespace msa { namespace event {

	typedef struct handler_context_type {
		const Event *event;
		EventHandler handler_func;
		HandlerSync *sync;
		bool running;
		pthread_t thread;
	} HandlerContext;

	struct event_dispatch_context_type {
		pthread_t edt;
		pthread_mutex_t queue_mutex;
		HandlerContext *current_handler;
		std::priority_queue<const Event *> queue;
		std::map<Topic, EventHandler> handlers;
		std::stack<HandlerContext *> interrupted;
	};

	static int create_event_dispatch_context(EventDispatchContext **event);
	static int dispose_event_dispatch_context(EventDispatchContext *event);
	static void *event_start(void *args);

	static const Event *peek_event(msa::Handle msa);
	static const Event *pop_event(msa::Handle msa);
	static void push_event(msa::Handle msa, const Event *e);

	static void *edt_start(void *args);
	static void edt_run(msa::Handle hdl);
	static void edt_cleanup(msa::Handle hdl);
	static const Event *edt_poll_event_queue(msa::Handle hdl);
	static void edt_interrupt_handler(msa::Handle hdl);
	static void edt_spawn_handler(msa::Handle hdl, const Event *e);
	static void edt_dispatch_event(msa::Handle hdl, const Event *e);
	static void dispose_handler_context(HandlerContext *ctx, bool join);

	extern int init(msa::Handle hdl)
	{
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
		return 0;
	}

	extern int quit(msa::Handle msa)
	{
		if (msa->status == msa::Status::CREATED && msa->event != NULL)
		{
			// this shouldn't happen, but if we get here, it's because
			// the event handle was inited but the EDT was not started.
			// We can just destroy the mutex and delete everything immediately
			pthread_mutex_destroy(&msa->event->queue_mutex);
			dispose_event_dispatch_context(msa->event);
		}
		// if the quit was initiated by the current event thread,
		// we must mark it as such so that the EDT knows not to
		// join on it (since this thread also joins on the EDT,
		// this would cause deadlock)
		if (pthread_self() == msa->event->current_handler->thread)
		{
			set_handler_syscall_origin(msa->event->current_handler->sync);
		}
		msa->status = msa::Status::STOP_REQUESTED;
		pthread_join(msa->event->edt, NULL);
		dispose_event_dispatch_context(msa->event);
		return 0;
	}

	extern void subscribe(msa::Handle msa, Topic t, EventHandler handler)
	{
		msa->event->handlers[t] = handler;
	}

	extern void unsubscribe(msa::Handle msa, Topic t, EventHandler handler)
	{
		msa->event->handlers[t] = NULL;
	}

	extern void generate(msa::Handle msa, Topic t, void *args)
	{
		const Event *e = create(t, args);
		push_event(msa, e);
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
		// it would seem that we should destory our queue mutex here, but
		// that is instead handled in the edt_cleanup function.
		delete event;
		return 0;
	}

	static void *edt_start(void *args)
	{
		int retval; // include for portability according to man page
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &retval);
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &retval);
		msa::Handle hdl = (msa::Handle) args;
		hdl->status = msa::Status::RUNNING;
		while (hdl->status != msa::Status::STOP_REQUESTED)
		{
			edt_run(hdl);
		}
		edt_cleanup(hdl);
	}

	static void edt_cleanup(msa::Handle hdl)
	{
		EventDispatchContext *ctx = hdl->event;
		if (ctx->current_handler != NULL)
		{
			// if the syscall that caused the EDT to enter cleanup
			// is from the current event handler, do not wait for it
			// to complete before freeing its resources
			bool join = !handler_syscall_origin(ctx->current_handler->sync);
			dispose_handler_context(ctx->current_handler, join);
		}
		while (!ctx->interrupted.empty())
		{
			HandlerContext *intr_ctx = ctx->interrupted.top();
			ctx->interrupted.pop();
			dispose_handler_context(intr_ctx, true);
		}
		pthread_mutex_destroy(&hdl->event->queue_mutex);
		while (!hdl->event->queue.empty())
		{
			const Event *e = hdl->event->queue.top();
			hdl->event->queue.pop();
			delete e;
		}
		hdl->status = msa::Status::STOPPED;
	}

	static void edt_run(msa::Handle hdl) {
		// check event_queue, decide if we want the current top
		const Event *e = edt_poll_event_queue(hdl);
		if (e != NULL)
		{
			edt_dispatch_event(hdl, e);
		}
		
		EventDispatchContext *edc = hdl->event;
		// check if current task has finished
		if (edc->current_handler != NULL && !(edc->current_handler->running))
		{
			dispose_handler_context(edc->current_handler, false);
			edc->current_handler = NULL;
		}
		// if current task is clear, load up the next one that has been interrupted
		if (edc->current_handler == NULL && !edc->interrupted.empty())
		{
			HandlerContext *ctx = edc->interrupted.top();
			edc->interrupted.pop();
			resume_handler(ctx->sync);
			edc->current_handler = ctx;
		}
		// TODO: Add a synthetic event for when queue has emptied and no events
	}

	static const Event *edt_poll_event_queue(msa::Handle hdl)
	{
		const Event *e = NULL;
		pthread_mutex_lock(&hdl->event->queue_mutex);
		if (!hdl->event->queue.empty())
		{
			e = hdl->event->queue.top();
			// if we have a current event, check to see if we should replace it
			// with the event on the queue
			if (hdl->event->current_handler != NULL)
			{
				HandlerContext *ctx = hdl->event->current_handler;
				uint8_t cur_prior = get_priority(ctx->event);
				uint8_t new_prior = get_priority(e);
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

	static void edt_interrupt_handler(msa::Handle hdl)
	{
		HandlerContext *ctx = hdl->event->current_handler;
		suspend_handler(ctx->sync);
		// wait till it stops
		while (!handler_suspended(ctx->sync)) {
			// we just wait here until we can go
			msa::util::sleep_milli(10);
			// TODO: Force handler to stop if it takes too long
		}
		// okay now put it on the stack
		hdl->event->interrupted.push(ctx);
		// and clear the current
		hdl->event->current_handler = NULL;
	}

	static void edt_spawn_handler(msa::Handle hdl, const Event *e)
	{
		HandlerContext *new_ctx = new HandlerContext;
		new_ctx->event = e;
		new_ctx->handler_func = hdl->event->handlers[e->topic];
		create_handler_sync(&new_ctx->sync);
		hdl->event->current_handler = new_ctx;
		new_ctx->running = (pthread_create(&new_ctx->thread, NULL, event_start, hdl) == 0);
	}

	static void edt_dispatch_event(msa::Handle hdl, const Event *e)
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

	static void dispose_handler_context(HandlerContext *ctx, bool join)
	{
		if (ctx->running)
		{
			if (handler_suspended(ctx->sync))
			{
				resume_handler(ctx->sync);
			}
			if (join)
			{
				// wait until current event runs through
				int err = pthread_join(ctx->thread, NULL);
			}
		}
		// delete event
		dispose(ctx->event);
		// delete sync handler
		dispose_handler_sync(ctx->sync);
		delete ctx;
	}

	void *event_start(void *args)
	{
		msa::Handle hdl = (msa::Handle) args;
		HandlerContext *ctx = hdl->event->current_handler;
		ctx->handler_func(hdl, ctx->event, ctx->sync);
		ctx->running = false;
	}

	static void push_event(msa::Handle msa, const Event *e)
	{
		pthread_mutex_lock(&msa->event->queue_mutex);
		msa->event->queue.push(e);
		pthread_mutex_unlock(&msa->event->queue_mutex);
	}

	static const Event *peek_event(msa::Handle msa)
	{
		pthread_mutex_lock(&msa->event->queue_mutex);
		const Event *e = msa->event->queue.top();
		pthread_mutex_unlock(&msa->event->queue_mutex);
		return e;
	}

	static const Event *pop_event(msa::Handle msa)
	{
		pthread_mutex_lock(&msa->event->queue_mutex);
		const Event *e = msa->event->queue.top();
		msa->event->queue.pop();
		pthread_mutex_unlock(&msa->event->queue_mutex);
	}

} }
