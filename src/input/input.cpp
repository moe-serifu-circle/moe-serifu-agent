#include "input/input.hpp"
#include "event/dispatch.hpp"
#include "util/util.hpp"
#include "log/log.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "platform/thread/thread.hpp"

namespace msa { namespace input {

	static const PluginHooks HOOKS = {
		#define MSA_MODULE_HOOK(retspec, name, ...)		name,
		#include "input/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};

	typedef Chunk *(*GetInputFunc)(msa::Handle, Device *);
	typedef bool (*CheckReadyFunc)(msa::Handle, Device *);

	typedef struct input_handler
	{
		GetInputFunc get_input;
		CheckReadyFunc is_ready;
	} InputHandler;

	static std::map<std::string, InputType> INPUT_TYPE_NAMES;
	static std::map<InputType, std::string> INPUT_TYPE_STRS;
	static std::map<std::string, InputHandler *> INPUT_HANDLER_NAMES;

	struct device_type
	{
		std::string id;
		InputType type;
		msa::thread::Thread thread;
		bool running;
		bool reap_in_runner;
		union
		{
			uint16_t port;
			const std::string *device_name;
		};
	};

	struct input_context_type
	{
		std::map<std::string, Device *> devices;
		std::vector<std::string> active;
		std::map<InputType, InputHandler *> handlers;
	};

	typedef struct it_args_type
	{
		msa::Handle hdl;
		Device *dev;
	} InputThreadArgs;

	static int init_static_resources();
	static int destroy_static_resources();
	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static void create_device(Device **dev, InputType type, const void *device_id);
	static void dispose_device(Device *device);
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);

	static Chunk *get_tty_input(msa::Handle hdl, Device *dev);
	static bool tty_ready(msa::Handle hdl, Device *dev);

	// input thread funcs
	static void *it_start(void *hdl);
	static InputHandler *it_get_handler(msa::Handle hdl, Device *dev);
	static void it_read_input(msa::Handle hdl, Device *dev, InputHandler *input_handler);
	static void it_cleanup(msa::Handle hdl, Device *dev);

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
	{
		init_static_resources();
		int stat = create_input_context(&hdl->input);
		if (stat != 0)
		{
			msa::log::error(hdl, "Could not create input context (error " + std::to_string(stat) + ")");
			return 1;
		}
		try
		{
			read_config(hdl, config);
		}
		catch (const msa::cfg::config_error &e)
		{
			msa::log::error(hdl, "Could not read input module config: " + std::string(e.what()));
			return 2;
		}
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		int status = dispose_input_context(hdl->input);
		if (status == 0)
		{
			hdl->input = NULL;
		}
		else
		{
			msa::log::error(hdl, "Could not dispose input context (error " + std::to_string(status) + ")");
		}
		destroy_static_resources(); // TODO: This will make it so only one instance of MSA can run at once!
		return status;
	}

	extern const PluginHooks *get_plugin_hooks()
	{
		return &HOOKS;
	}
	
	extern void add_device(msa::Handle hdl, InputType type, void *device_id)
	{
		Device *dev;
		create_device(&dev, type, device_id);
		const std::string &id = dev->id;
		if (hdl->input->devices.find(id) != hdl->input->devices.end())
		{
			dispose_device(dev);
			throw std::logic_error("input device already exists: " + id);
		}
		hdl->input->devices[id] = dev;
	}

	extern void remove_device(msa::Handle hdl, const std::string &id)
	{
		if (hdl->input->devices.find(id) == hdl->input->devices.end())
		{
			throw std::logic_error("input device does not exist: " + id);
		}
		Device *dev = hdl->input->devices[id];
		if (dev->running)
		{
			// mark it as collectable by the calling thread
			dev->reap_in_runner = true;
			disable_device(hdl, id);
		}
		else
		{
			// no input_thread to take care of it, delete it ourselves
			dispose_device(dev);
		}
		hdl->input->devices.erase(id);
	}

	extern void get_devices(msa::Handle hdl, std::vector<std::string> *list)
	{
		std::map<std::string, Device *> *devs = &hdl->input->devices;
		typedef std::map<std::string, Device *>::iterator it_type;
		for (it_type iter = devs->begin(); iter != devs->end(); iter++)
		{
			std::string id = iter->second->id;
			list->push_back(id);
		}
	}

	extern void enable_device(msa::Handle hdl, const std::string &id)
	{
		std::vector<std::string> &act = hdl->input->active;
		if (std::find(act.begin(), act.end(), id) != act.end())
		{
			// it's already active, leave it alone
			return;
		}
		if (hdl->input->devices.find(id) == hdl->input->devices.end())
		{
			// device does not exist
			throw std::logic_error("input device does not exist: " + id);
		}
		// checks are done, we know it exists and is disabled
		Device *dev = hdl->input->devices[id];
		InputThreadArgs *ita = new InputThreadArgs;
		ita->dev = dev;
		ita->hdl = hdl;

		msa::thread::Attributes attr;
		msa::thread::attr_init(&attr);
		msa::thread::attr_set_detach(&attr, true);
		bool started = (msa::thread::create(&dev->thread, &attr, it_start, ita, "input") == 0);
		msa::thread::attr_destroy(&attr);
		
		if (started)
		{
			hdl->input->active.push_back(dev->id);
			msa::log::info(hdl, "Enabled input device " + dev->id);
		}
		else
		{
			msa::log::warn(hdl, "Could not enable input device " + dev->id);
		}
	}

	extern void disable_device(msa::Handle hdl, const std::string &id)
	{
		std::vector<std::string> &act = hdl->input->active;
		if (std::find(act.begin(), act.end(), id) == act.end())
		{
			// it's not enabled, leave it alone.
			return;
		}
		if (hdl->input->devices.find(id) == hdl->input->devices.end())
		{
			// device does not exist
			throw std::logic_error("input device does not exist: " + id);
		}
		hdl->input->devices[id]->running = false;
		act.erase(std::find(act.begin(), act.end(), id));
		msa::log::info(hdl, "Disabled input device " + id);
	}

	static int init_static_resources()
	{
		if (INPUT_TYPE_NAMES.empty())
		{
			INPUT_TYPE_NAMES["UDP"] = InputType::UDP;
			INPUT_TYPE_NAMES["TCP"] = InputType::TCP;
			INPUT_TYPE_NAMES["TTY"] = InputType::TTY;
		}
		if (INPUT_TYPE_STRS.empty())
		{
			INPUT_TYPE_STRS[InputType::UDP] = "UDP";
			INPUT_TYPE_STRS[InputType::TCP] = "TCP";
			INPUT_TYPE_STRS[InputType::TTY] = "TTY";
		}
		if (INPUT_HANDLER_NAMES.empty())
		{
			INPUT_HANDLER_NAMES["get_tty_input"] = new InputHandler {get_tty_input, tty_ready};
		}
		return 0;
	}

	static int destroy_static_resources()
	{
		for (auto it = INPUT_HANDLER_NAMES.begin(); it != INPUT_HANDLER_NAMES.end(); it++)
		{
			delete it->second;
		}
		INPUT_HANDLER_NAMES.clear();
		return 0;
	}

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		if (config.has("TYPE") && config.has("ID") && config.has("HANDLER"))
		{
			// for each of the configs, read it in
			const std::vector<InputType> types = config.get_all_as_enum("TYPE", INPUT_TYPE_NAMES);
			const std::vector<std::string> ids = config.get_all("ID");
			const std::vector<InputHandler*> handlers = config.get_all_as_enum("HANDLER", INPUT_HANDLER_NAMES);
			for (size_t i = 0; i < types.size() && i < ids.size() && i < handlers.size(); i++)
			{
				std::string id = ids[i];
				InputType type = types[i];
				InputHandler *handler = handlers[i];
				hdl->input->handlers[type] = handler;
				add_device(hdl, type, &id);
				enable_device(hdl, INPUT_TYPE_STRS[type] + ":" + id);
			}
		}
	}

	static void create_device(Device **dev_ptr, InputType type, const void *id)
	{
		Device *dev = new Device;
		dev->running = false;
		dev->reap_in_runner = false;
		dev->type = type;
		switch (dev->type)
		{
			case InputType::TCP:
				dev->port = *(static_cast<const uint16_t *>(id));
				dev->id = "TCP:" + std::to_string(dev->port);
				break;

			case InputType::UDP:
				dev->port = *(static_cast<const uint16_t *>(id));
				dev->id = "UDP:" + std::to_string(dev->port);
				break;

			case InputType::TTY:
				dev->device_name = new std::string(*static_cast<const std::string *>(id));
				dev->id = "TTY:" + *dev->device_name;
				break;

			default:
				delete dev;
				throw std::invalid_argument("unknown input type: " + std::to_string(type));
				break;
		}
		*dev_ptr = dev;
	}

	static void dispose_device(Device *dev)
	{	
		if (dev->running)
		{
			dev->running = false;
		}
		if (dev->type == InputType::TTY)
		{
			delete dev->device_name;
		}
		delete dev;
	}

	static int create_input_context(InputContext **ctx)
	{
		InputContext *io_ctx = new InputContext;
		*ctx = io_ctx;
		return 0;
	}

	static int dispose_input_context(InputContext *ctx)
	{
		typedef std::map<std::string, Device *>::iterator it_type;
		it_type iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			Device *dev = iter->second;
			if (dev->running)
			{
				// let input_thread take care of deleting it
				dev->reap_in_runner = true;
				dev->running = false;
				std::vector<std::string> &act = ctx->active;
				act.erase(std::find(act.begin(), act.end(), dev->id));
			}
			else
			{
				dispose_device(iter->second);
			}
			iter = ctx->devices.erase(iter);
		}
		delete ctx;
		return 0;
	}

	static void *it_start(void *args)
	{
		InputThreadArgs *ita = static_cast<InputThreadArgs *>(args);
		msa::Handle hdl = ita->hdl;
		Device *dev = ita->dev;
		delete ita;

		dev->running = true;
		
		InputHandler *input_handler = it_get_handler(hdl, dev);

		msa::log::info(hdl, "Started reading from input device " + dev->id);
		it_read_input(hdl, dev, input_handler);
		msa::log::info(hdl, "Stopped reading from input device " + dev->id);

		it_cleanup(hdl, dev);
		
		return NULL;
	}

	static InputHandler *it_get_handler(msa::Handle hdl, Device *dev)
	{
		if (hdl->input->handlers.find(dev->type) == hdl->input->handlers.end())
		{
			dev->running = false;
			disable_device(hdl, dev->id);
			throw std::logic_error("no handler for input device type " + std::to_string(dev->type));
		}
		return hdl->input->handlers[dev->type];
	}

	static void it_read_input(msa::Handle hdl, Device *dev, InputHandler *input_handler)
	{
		while (dev->running)
		{
			if (input_handler->is_ready(hdl, dev))
			{
				Chunk *chunk = input_handler->get_input(hdl, dev);
				msa::log::trace(hdl, "Got input; notifying event system");
				msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, msa::event::wrap(chunk->text));
				msa::log::trace(hdl, "Input event has been pushed to the queue");
				delete chunk;
			}
		}
	}

	static void it_cleanup(msa::Handle hdl, Device *dev)
	{
		if (dev->reap_in_runner)
		{
			msa::log::info(hdl, "Freeing input device " + dev->id);
			dispose_device(dev);
		}
	}

	static Chunk *get_tty_input(msa::Handle UNUSED(hdl), Device *UNUSED(dev))
	{
		std::string input;
		std::getline(std::cin, input);
		Chunk *ch = new Chunk;
		ch->text = input;
		return ch;
	}

	static bool tty_ready(msa::Handle UNUSED(hdl), Device *UNUSED(dev))
	{
		return msa::util::check_stdin_ready();
	}

} }
