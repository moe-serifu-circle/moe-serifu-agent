#include "output/output.hpp"
#include "log/log.hpp"
#include "util/string.hpp"

#include <map>
#include <stdexcept>
#include <algorithm>

#include "platform/thread/thread.hpp"

namespace msa { namespace output {

	static const PluginHooks HOOKS = {
		#define MSA_MODULE_HOOK(retspec, name, ...)		name,
		#include "output/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
	
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
			const std::string *device_name;
		};
	};

	struct output_context_type
	{
		msa::thread::Mutex *state_mutex;
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

	static std::map<std::string, OutputType> OUTPUT_TYPE_NAMES;
	static OutputHandler *default_stdout_handler = NULL;

	static void print_to_stdout(msa::Handle hdl, const Chunk *chunk, Device *dev);

	static int create_output_context(OutputContext **ctx);
	static int dispose_output_context(OutputContext *ctx);
	static int create_device(Device **dev_ptr, OutputType type, const OutputHandler *handler, const void *id);
	static int dispose_device(Device *dev);
	static bool handler_is_registered(msa::Handle hdl, OutputType type, const std::string &name);
	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static void switch_to_next_device(msa::Handle hdl, const std::vector<std::string> &bad_ids);
	static void switch_device_internal(msa::Handle hdl, const std::string &id);
	static void create_default_handlers(msa::Handle hdl);
	static void dispose_default_handlers(msa::Handle hdl);
	static int init_static_resources();

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
	{
		init_static_resources();
		if (create_output_context(&hdl->output) != 0)
		{
			msa::log::error(hdl, "Could not create output context");
			return -1;
		}
		create_default_handlers(hdl);
		try
		{
			read_config(hdl, config);
		}
		catch (const msa::cfg::config_error &e)
		{
			msa::log::error(hdl, "Could not read output module config: " + std::string(e.what()));
			return -3;
		}
		// has to be at least one device
		if (hdl->output->active == "")
		{
			msa::log::error(hdl, "No active output device");
			return -2;
		}
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		hdl->output->active = "";
		dispose_default_handlers(hdl);
		int status = dispose_output_context(hdl->output);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not dispose output context (error " + std::to_string(status) + ")");
			return -1;
		}
		return 0;
	}

	extern const PluginHooks *get_plugin_hooks()
	{
		return &HOOKS;
	}
	
	extern void write(msa::Handle hdl, const Chunk *chunk)
	{
		OutputContext *ctx = hdl->output;
		if (hdl->status == msa::Status::RUNNING && ctx != NULL && ctx->running)
		{
			msa::thread::mutex_lock(ctx->state_mutex);
			Device *dev = ctx->devices[ctx->active];
			try
			{
				dev->handler->func(hdl, chunk, dev);
			}
			catch (...)
			{
				msa::thread::mutex_unlock(ctx->state_mutex);
				throw;
			}
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
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		if (!handler_is_registered(hdl, type, handler_id))
		{
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::logic_error("handler does not exist for output type " + std::to_string(type) + ": " + handler_id);
		}
		Device *dev;
		create_device(&dev, type, ctx->handlers[type][handler_id], device_id);
		const std::string &id = dev->id;
		if (ctx->devices.find(id) != ctx->devices.end())
		{
			dispose_device(dev);
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw std::invalid_argument("output device already exists: " + dev->id);
		}
		ctx->devices[id] = dev;
		if (ctx->active == "")
		{
			switch_device_internal(hdl, id);
		}
		msa::thread::mutex_unlock(ctx->state_mutex);
		msa::log::info(hdl, "Added output device " + id);
	}
	
	extern void get_devices(msa::Handle hdl, std::vector<std::string> *list)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(hdl->output->state_mutex);
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
			std::vector<std::string> bad_ids;
			bad_ids.push_back(id);
			try
			{
				switch_to_next_device(hdl, bad_ids);
			}
			catch (...)
			{
				msa::thread::mutex_unlock(ctx->state_mutex);
				throw;
			}
		}
		dispose_device(ctx->devices[id]);
		ctx->devices.erase(id);
		msa::thread::mutex_unlock(ctx->state_mutex);
		msa::log::info(hdl, "Removed output device " + id);
	}
	
	extern void switch_device(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->state_mutex);
		try
		{
			switch_device_internal(hdl, id);
		}
		catch (...)
		{
			msa::thread::mutex_unlock(ctx->state_mutex);
			throw;
		}
		msa::thread::mutex_unlock(ctx->state_mutex);
		msa::log::info(hdl, "Switched to input device " + id);
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
		if (typed_handlers.find(handler->name) != typed_handlers.end())
		{
			throw std::logic_error("output handler already exists: " + std::to_string(type) + "/" + handler->name);
		}
		typed_handlers[handler->name] = handler;
	}
	
	extern void unregister_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler)
	{
		OutputContext *ctx = hdl->output;
		if (!handler_is_registered(hdl, type, handler->name))
		{
			return;
		}
		msa::thread::mutex_lock(ctx->state_mutex);
		if (ctx->active != "")
		{
			Device *active_dev = ctx->devices[ctx->active];
			std::vector<std::string> bad_ids;
			while (active_dev->type == type && active_dev->handler->name == handler->name)
			{
				bad_ids.push_back(ctx->active);
				try
				{
					switch_to_next_device(hdl, bad_ids);
				}
				catch (...)
				{
					msa::thread::mutex_unlock(ctx->state_mutex);
					throw;
				}
				active_dev = ctx->devices[ctx->active];
			}
		}
		ctx->handlers[type].erase(handler->name);
		msa::thread::mutex_unlock(ctx->state_mutex);
	}
	
	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		if (config.has("TYPE") && config.has("HANDLER") && config.has("ID"))
		{
			const std::vector<OutputType> types = config.get_all_as_enum("TYPE", OUTPUT_TYPE_NAMES);
			const std::vector<std::string> handlers = config.get_all("HANDLER");
			const std::vector<std::string> ids = config.get_all("ID");
			for (size_t i = 0; i < types.size() && i < handlers.size() && i < ids.size(); i++)
			{
				OutputType type = types[i];
				std::string handler_str = handlers[i];
				std::string id_str = ids[i];
				if (!handler_is_registered(hdl, type, handler_str))
				{
					throw msa::cfg::config_error(config.get_name(), "HANDLER", i, handler_str, "no OutputType::" + std::to_string(type) + " handler registered as that");
				}
				void *id;
				uint16_t port = 0;
				// select what ID should point to based on type
				if (type == OutputType::UDP || type == OutputType::TCP)
				{
					port = (uint16_t) std::stoi(id_str);
					id = &port;
				}
				else
				{
					id = &id_str;
				}
				add_device(hdl, type, handler_str, id);
			}
		}
		if (hdl->output->active == "")
		{
			msa::log::warn(hdl, "no active output devices read from config");
		}
	}

	static int create_output_context(OutputContext **ctx)
	{
		OutputContext *output = new OutputContext;
		output->running = true;
		output->active = "";
		output->state_mutex = new msa::thread::Mutex;
		msa::thread::mutex_init(output->state_mutex, NULL);
		*ctx = output;
		return 0;
	}

	static int dispose_output_context(OutputContext *ctx)
	{
		ctx->active = "";
		ctx->running = false;
		std::map<std::string, Device *>::iterator iter;
		iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			iter->second->active = false;
			dispose_device(iter->second);
			iter = ctx->devices.erase(iter);
		}
		msa::thread::mutex_destroy(ctx->state_mutex);
		delete ctx->state_mutex;
		delete ctx;
		return 0;
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
				dev->device_name = new std::string(*static_cast<const std::string *>(id));
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
	
	static void switch_to_next_device(msa::Handle hdl, const std::vector<std::string> &bad_ids)
	{
		OutputContext *ctx = hdl->output;
		// need to find ID of the next input device
		std::string next_id = "";
		std::map<std::string, Device *>::const_iterator iter;
		for (iter = ctx->devices.begin(); iter != ctx->devices.end(); iter++)
		{
			if (std::find(bad_ids.begin(), bad_ids.end(), iter->second->id) == bad_ids.end())
			{
				next_id = iter->second->id;
				break;
			}
		}
		// if we didn't find a device to switch to, do not switch
		if (next_id == "")
		{
			throw std::logic_error("no valid output device to switch to");
		}
		switch_device_internal(hdl, next_id);
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

	static void switch_device_internal(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		if (ctx->devices.find(id) == ctx->devices.end())
		{
			throw std::invalid_argument("output device does not exist: " + id);
		}
		if (ctx->active != "")
		{
			ctx->devices[ctx->active]->active = false;
		}
		ctx->active = id;
		ctx->devices[id]->active = true;
	}

	static void print_to_stdout(msa::Handle UNUSED(hdl), const Chunk *ch, Device *dev)
	{
		if (*dev->device_name != "STDOUT")
		{
			throw std::logic_error("cannot print to TTY terminal: " + *dev->device_name);
		}
		printf("%s", ch->text->c_str());
	}

	static void create_default_handlers(msa::Handle hdl)
	{
		if (default_stdout_handler == NULL)
		{
			create_handler(&default_stdout_handler, "print_to_stdout", print_to_stdout);
		}
		register_handler(hdl, OutputType::TTY, default_stdout_handler);
	}

	static void dispose_default_handlers(msa::Handle hdl)
	{
		unregister_handler(hdl, OutputType::TTY, default_stdout_handler);
		dispose_handler(default_stdout_handler); // TODO: need to move to quit() in init/start/stop/quit model
	}

	static int init_static_resources()
	{
		if (OUTPUT_TYPE_NAMES.empty())
		{
			OUTPUT_TYPE_NAMES["UDP"] = OutputType::UDP;
			OUTPUT_TYPE_NAMES["TCP"] = OutputType::TCP;
			OUTPUT_TYPE_NAMES["TTY"] = OutputType::TTY;
		}
		return 0;
	}

} }
