#include "event/dispatch.hpp"
#include "util.hpp"
#include "log/log.hpp"

#include <cstdio>
#include <queue>
#include <stack>
#include <map>
#include <string>
#include <exception>

#include "platform/thread/thread.hpp"

namespace msa { namespace event {

	static const PluginHooks HOOKS = {
		#define MSA_MODULE_HOOK(retspec, name, ...)		name,
		#include "event/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};

	typedef struct handler_context_type {
		const Event *event;
		EventHandler handler_func;
		HandlerSync *sync;
		bool running;
		msa::thread::Thread thread;
		bool reap_in_handler;
	} HandlerContext;

	struct event_dispatch_context_type {
		msa::thread::Thread edt;
		msa::thread::Mutex queue_mutex;
		HandlerContext *current_handler;
		std::priority_queue<const Event *> queue;
		std::map<Topic, EventHandler> handlers;
		std::stack<HandlerContext *> interrupted;
		int sleep_time;
	};

	static int create_event_dispatch_context(EventDispatchContext **event);
	static int dispose_event_dispatch_context(EventDispatchContext *event);
	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static void *event_start(void *args);
	
	static void push_event(msa::Handle msa, const Event *e);

	static void *edt_start(void *args);
	static void edt_run(msa::Handle hdl);
	static void edt_cleanup(msa::Handle hdl);
	static const Event *edt_poll_event_queue(msa::Handle hdl);
	static void edt_interrupt_handler(msa::Handle hdl);
	static void edt_spawn_handler(msa::Handle hdl, const Event *e);
	static void edt_dispatch_event(msa::Handle hdl, const Event *e);
	static void dispose_handler_context(HandlerContext *ctx, bool wait);

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
	{
		int create_status = create_event_dispatch_context(&hdl->event);
		if (create_status != 0)
		{
			msa::log::error(hdl, "Could not create event context (error " + std::to_string(create_status) + ")");
			return create_status;
		}
		
		// read config
		try
		{
			read_config(hdl, config);
		}
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read event config: " + std::string(e.what()));
		}

		create_status = msa::thread::create(&hdl->event->edt, NULL, edt_start, hdl, "edt");
		if (create_status != 0)
		{
			msa::log::error(hdl, "Could not create event dispatch thread (error " + std::to_string(create_status) + ")");
			msa::thread::mutex_destroy(&hdl->event->queue_mutex);
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
			msa::log::warn(msa, "EDT has not yet set status to RUNNING! Killing anyways");
			msa::thread::mutex_destroy(&msa->event->queue_mutex);
			dispose_event_dispatch_context(msa->event);
		}
		// if the quit was initiated by the current event thread,
		// we must mark it as such so that the EDT knows not to
		// wait on it (since this thread also joins on the EDT,
		// this would cause deadlock)
		if (msa::thread::self() == msa->event->current_handler->thread)
		{
			set_handler_syscall_origin(msa->event->current_handler->sync);
		}
		msa->status = msa::Status::STOP_REQUESTED;
		msa::log::trace(msa, "Joining on EDT");
		msa::thread::join(msa->event->edt, NULL);
		msa::log::trace(msa, "EDT joined");
		dispose_event_dispatch_context(msa->event);
		return 0;
	}

	extern const PluginHooks *get_plugin_hooks()
	{
		return &HOOKS;
	}

	extern void subscribe(msa::Handle msa, Topic t, EventHandler handler)
	{
		msa->event->handlers[t] = handler;
	}

	extern void unsubscribe(msa::Handle msa, Topic t, EventHandler UNUSED(handler))
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
		msa::thread::mutex_init(&edc->queue_mutex, NULL);
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

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		hdl->event->sleep_time = std::stoi(config.get_or("IDLE_SLEEP_TIME", "10"));
	}

	static void *edt_start(void *args)
	{
		msa::Handle hdl = (msa::Handle) args;
		hdl->status = msa::Status::RUNNING;
		while (hdl->status != msa::Status::STOP_REQUESTED)
		{
			edt_run(hdl);
			msa::util::sleep_milli(hdl->event->sleep_time);
		}
		edt_cleanup(hdl);
		return NULL;
	}

	static void edt_cleanup(msa::Handle hdl)
	{
		EventDispatchContext *ctx = hdl->event;
		if (ctx->current_handler != NULL)
		{
			// if the syscall that caused the EDT to enter cleanup
			// is from the current event handler, do not wait for it
			// to complete before freeing its resources
			bool wait = !handler_syscall_origin(ctx->current_handler->sync);
			dispose_handler_context(ctx->current_handler, wait);
		}
		while (!ctx->interrupted.empty())
		{
			HandlerContext *intr_ctx = ctx->interrupted.top();
			ctx->interrupted.pop();
			dispose_handler_context(intr_ctx, true);
		}
		msa::thread::mutex_destroy(&hdl->event->queue_mutex);
		while (!hdl->event->queue.empty())
		{
			const Event *e = hdl->event->queue.top();
			hdl->event->queue.pop();
			delete e;
		}
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
		msa::thread::mutex_lock(&hdl->event->queue_mutex);
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
		msa::thread::mutex_unlock(&hdl->event->queue_mutex);
		return e;
	}

	static void edt_interrupt_handler(msa::Handle hdl)
	{
		HandlerContext *ctx = hdl->event->current_handler;
		suspend_handler(ctx->sync);
		// wait till it stops
		while (!handler_suspended(ctx->sync)) {
			// we just wait here until we can go
			msa::util::sleep_milli(hdl->event->sleep_time);
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
		new_ctx->reap_in_handler = false;
		new_ctx->event = e;
		new_ctx->handler_func = hdl->event->handlers[e->topic];
		create_handler_sync(&new_ctx->sync);
		hdl->event->current_handler = new_ctx;
		
		msa::thread::Attributes *attr = new msa::thread::Attributes;
		msa::thread::attr_init(attr);
		msa::thread::attr_set_detach(attr, true);
		new_ctx->running = (msa::thread::create(&new_ctx->thread, attr, event_start, hdl, "handler") == 0);
		msa::thread::attr_destroy(attr);
		delete attr;
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

	static void dispose_handler_context(HandlerContext *ctx, bool wait)
	{
		if (ctx->running)
		{
			if (handler_suspended(ctx->sync))
			{
				resume_handler(ctx->sync);
			}
			if (wait)
			{
				// wait until current event runs through
				while (ctx->running)
				{
					msa::util::sleep_milli(10);
				}
			}
			else
			{
				ctx->reap_in_handler = true;
			}
		}
		if (!ctx->reap_in_handler)
		{
			// delete event
			dispose(ctx->event);
			// delete sync handler
			dispose_handler_sync(ctx->sync);
			delete ctx;
		}
	}

	void *event_start(void *args)
	{
		msa::Handle hdl = (msa::Handle) args;
		HandlerContext *ctx = hdl->event->current_handler;
		ctx->handler_func(hdl, ctx->event, ctx->sync);
		if (ctx->reap_in_handler)
		{
			dispose(ctx->event);
			dispose_handler_sync(ctx->sync);
			delete ctx;
		}
		else
		{
			ctx->running = false;
		}
		return NULL;
	}

	static void push_event(msa::Handle msa, const Event *e)
	{
		msa::thread::mutex_lock(&msa->event->queue_mutex);
		msa->event->queue.push(e);
		msa::thread::mutex_unlock(&msa->event->queue_mutex);
	}

} }
