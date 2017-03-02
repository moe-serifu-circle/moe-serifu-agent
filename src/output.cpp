#include "output.hpp"
#include "log.hpp"

#include <map>
#include <stdexcept>

namespace msa { namespace output {
	
	typedef std::map<std::string, const OutputHandler *> TypedHandlerMap;
	typedef std::map<OutputType, TypedHandlerMap> HandlerMap;

	struct chunk_type
	{
		std::string *text;
	};

	struct device_type
	{
		std::string id;
		const OutputHandler *handler;
		OutputType type;
		bool active;
		union
		{
			uint16_t port;
			const std::string device_name;
		};
	};

	struct output_context_type
	{
		msa::thread::Mutex *state_mutex;
		msa::thread::Mutex *output_mutex;
		std::map<std::string, Device *> devices;
		std::string active;
		bool running;
		HandlerMap handlers;
	};

	struct output_handler_type
	{
		std::string name;
		OutputHandlerFunc func;
	};

	static void print_to_tty(msa::Handle hdl, Chunk *chunk, Device *dev);

	static int create_output_context(OutputContext **ctx);
	static int dispose_output_context(OutputContext *ctx);
	static int create_device(Device **dev_ptr, OutputType type, const OutputHandler *handler, const void *id);
	static int dispose_device(Device *dev);
	static bool handler_is_registered(msa::Handle hdl, OutputType type, const std::string &name);
	static void read_config(msa::Handle hdl, msa::config::Section &config);

	// note: usage mutex MUST be locked when calling this function
	static void switch_to_next_device(msa::Handle hdl);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		if (!create_output_context(&hdl->output))
		{
			return -1;
		}
		read_config(hdl, config);
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		if (!dispose_output_context(hdl->output))
		{
			return -1;
		}
		return 0;
	}
	
	extern void write(msa::Handle hdl, const Chunk *chunk)
	{
		OutputContext *ctx = hdl->output;
		if (ctx != NULL && ctx->running)
		{
			msa::thread::mutex_lock(ctx->state_mutex);
			
			msa::thread::mutex_unlock(ctx->state_mutex);
		}
	}

	extern void write_text(msa::Handle hdl, const std::string &text)
	{
		Chunk *ch;
		create_chunk(&ch, text);
		write(hdl, ch);
		dispose_chunk(ch);
	}

	extern void add_device(msa::Handle hdl, OutputType type, const std::string &handler_id, void *device_id)
	{
		OutputContex *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		if (!handler_is_registered(hdl, type, handler_id))
		{
			throw std::logic_error("handler does not exist for output type " + std::to_string(type) + ": " + handler_id);
		}
		Device *dev;
		create_device(&dev, type, ctx->handlers[type][handler_id], device_id);
		const std::string &id = dev->id;
		if (ctx->devices.find(id) != ctx->devices.end())
		{
			dispose_device(dev);
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::invalid_argument("output device already exists: " + std::to_string(dev->id));
		}
		ctx->devices[id] = dev;
		msa::thread::mutex_unlock(ctx->state_mutex);
		msa::log::info("Added output device " + id);
	}
	
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list)
	{
		msa::thread::mutex_lock(hdl->output->state_mutex);
		std::map<std::string, Device *> *devs = *hdl->output->devices;
		std::map<std::string, Device *>::const_iterator iter;
		for (iter = ctx->devices.begin(); iter != ctx->devices.end(); iter++)
		{
			std::string id = iter->second->id;
			list->push_back(id);
		}
		msa::thread::mutex_unlock(hdl->output->state_mutex);
	}
	
	extern void remove_device(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		if (ctx->devices.find(id) == ctx->devices.end())
		{
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::invalid_argument("output device does not exist: " + id);
		}
		if (ctx->active == id)
		{
			switch_to_next_device(hdl);
		}
		dispose_device(ctx->devices[id]);
		ctx->devices.erase(id);
		msa::thread::mutex_unlock(ctx->state_mutex);
		msa::log::info("Removed output device " + id);
	}
	
	extern void switch_device(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		if (ctx->devices.find(id) == ctx->devices.end())
		{
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::invalid_argument("output device does not exist: " + id);
		}
		if (ctx->active != "")
		{
			ctx->devices[ctx->active]->active = false;
		}
		ctx->active = id;
		ctx->devices[id]->active = true;
		msa::thread::mutex_unlock(ctx->state_mutex);
	}

	extern void get_active_device(msa::Handle hdl, std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		id = ctx->active;
		msa::thread::mutex_unlock(ctx->state_mutex);
	}
	
	extern void create_chunk(Chunk **chunk, const std::string &text)
	{
		Chunk *ch = new Chunk;	
		ch->text = new std::string(text);
		*chunk = ch;
	}

	extern void dispose_chunk(Chunk *chunk)
	{
		delete chunk->text;
		delete chunk;
	}

	extern void create_handler(OutputHandler **handler_ptr, const std::string &name, OutputHandlerFunc func)
	{
		OutputHandler *handler = new OutputHandler;
		handler->name = name;
		handler->func = func;
		*handler_ptr = handler;
	}
	
	extern void dispose_handler(OutputHandler *handler)
	{
		delete handler;
	}
	
	extern const std::string &get_handler_name(const OutputHandler *handler)
	{
		return handler->name;
	}

	extern void register_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler)
	{
		OutputContext *ctx = hdl->output;
		HandlerMap &handlers = ctx->handlers;
		if (handlers.find(type) == handlers.end())
		{
			handlers[type] = std::map<std::string, const OutputHandler *>();
		}
		TypedHandlerMap &typed_handlers = handlers[type];
		if (typed_handlers.find(handler->name) == typed_handlers.end())
		{
			throw std::logic_error("output handler already exists: " + std::to_string(type) + "/" + handler->name);
		}
		typed_handlers[handler->name] = handler;
	}
	
	extern void unregister_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		if (!handler_is_registered(hdl, type, handler->name))
		{
			return;
		}
		if (ctx->active != "")
		{
			Device *active_dev = ctx->devices[ctx->active];
			while (active_dev->type == type && active_dev->handler->name == handler->name)
			{
				switch_to_next_device(hdl);
			}
		}
		ctx->handlers[type].erase(name);
	}

	static int create_output_context(OutputContext **ctx)
	{
		output = new OutputContext;
		ctx->running = true;
		output->state_mutex = new msa::thread::Mutex;
		output->output_mutex = new msa::thread::Mutex;
		msa::thread::mutex_init(output->state_mutex, NULL);
		msa::thread::mutex_init(output->output_mutex, NULL);
		*ctx = output;
		return 0;
	}

	static int dispose_output_context(OutputContext *ctx)
	{
		ctx->running = false;
		Device *dev = ctx->devices[ctx->active];
		dev->active = false;
		dispose_device(dev);
		ctx->active = "";
		msa::thread::mutex_destroy(output->state_mutex);
		msa::thread::mutex_destroy(output->output_mutex);
		delete output->state_mutex;
		delete output->output_mutex;
		delete ctx;
	}
	
	static int create_device(Device **dev_ptr, OutputType type, const OutputHandler *handler, const void *id)
	{
		Device *dev = new Device;
		dev->type = type;
		dev->active = false;
		dev->handler = handler;
		switch (type)
		{
			case OutputType::TCP:
				dev->port = *(static_cast<const uint16_t *>(id));
				dev->id = "TCP:" + std::to_string(dev->port);
				break;

			case OutputType::UDP:
				dev->port = *(static_cast<const uint16_t *>(id));
				dev->id = "UDP:" + std::to_string(dev->port);
				break;

			case OutputType::TTY:
				dev->device_name = static_cast<const std::string *>(id);
				dev->id = "TTY:" + *dev->device_name;
				break;

			default:
				delete dev;
				throw std::invalid_argument("unknown output type: " + std::to_string(type));
				break;
		}
		*dev_ptr = dev;
		return 0;
	}
	
	static int dispose_device(Device *dev)
	{
		if (dev->type == OutputType::TTY)
		{
			delete dev->device_name;
		}
		delete dev;
		return 0;
	}

	// Note: usage mutex MUST be locked when calling this function
	static void switch_to_next_device(msa::Handle hdl)
	{
		OutputContext *ctx = hdl->output;
		// need to find ID of the next input device
		std::string next_id = "";
		std::map<std::string, Device *>::const_iterator iter;
		for (iter = ctx->devices.begin(); iter != ctx->devices.end(); iter++)
		{
			if (iter->second->id != id)
			{
				next_id = iter->second->id;
				break;
			}
		}
		// if we didn't find a device to switch to, do not remove
		if (next_id == "")
		{
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::logic_error("cannot remove sole output device " + id);
		}
		// otherwise, switch out before removing
		msa::thread::mutex_unlock(ctx->state_mutex);
		switch_device(hdl, next_id);
		msa::thread::mutex_lock(ctx->state_mutex);
	}

	static bool handler_is_registered(msa::Handle hdl, OutputType type, const std::string &name)
	{
		HandlerMap handlers = hdl->output->handlers;
		if (handlers.find(type) == handlers.end())
		{
			return false;
		}
		return (handlers[type].find(name) != handlers[type].end());
	}

} }
