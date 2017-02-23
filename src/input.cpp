#include "input.hpp"
#include "event/dispatch.hpp"
#include "util.hpp"
#include "log.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "platform/thread/thread.hpp"

namespace msa { namespace input {

	typedef InputChunk *(*GetInputFunc)(msa::Handle, InputDevice *);
	typedef bool (*CheckReadyFunc)(msa::Handle, InputDevice *);

	typedef struct input_handler
	{
		GetInputFunc get_input;
		CheckReadyFunc is_ready;
	} InputHandler;

	static std::map<std::string, InputType> INPUT_TYPE_NAMES;
	static std::map<std::string, InputHandler *> INPUT_HANDLER_NAMES;

	struct input_chunk_type
	{
		std::string chars;
	};

	struct input_device_type
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
		std::map<std::string, InputDevice *> devices;
		std::vector<std::string> active;
		std::map<InputType, InputHandler *> handlers;
	};

	typedef struct it_args_type
	{
		msa::Handle hdl;
		InputDevice *dev;
	} InputThreadArgs;

	static int init_static_resources();
	static int destroy_static_resources();
	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static void create_input_device(InputDevice **dev, InputType type, const void *device_id);
	static void dispose_input_device(InputDevice *device);
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);

	static InputChunk *get_tty_input(msa::Handle hdl, InputDevice *dev);
	static bool tty_ready(msa::Handle hdl, InputDevice *dev);

	static void interpret_cmd(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	// input thread funcs
	static void *it_start(void *hdl);
	static InputHandler *it_get_handler(msa::Handle hdl, InputDevice *dev);
	static void it_read_input(msa::Handle hdl, InputDevice *dev, InputHandler *input_handler);
	static void it_cleanup(msa::Handle hdl, InputDevice *dev);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		init_static_resources();
		int stat = create_input_context(&hdl->input);
		if (stat != 0)
		{
			return 1;
		}
		read_config(hdl, config);
		msa::event::subscribe(hdl, msa::event::Topic::TEXT_INPUT, interpret_cmd);
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		int status = dispose_input_context(hdl->input);
		if (status == 0)
		{
			hdl->input = NULL;
		}
		destroy_static_resources(); // TODO: This will make it so only one instance of MSA can run at once!
		return status;
	}
	
	extern void add_input_device(msa::Handle hdl, InputType type, void *device_id)
	{
		InputDevice *dev;
		create_input_device(&dev, type, device_id);
		std::string id = dev->id;
		if (hdl->input->devices.find(id) != hdl->input->devices.end())
		{
			dispose_input_device(dev);
			throw std::logic_error("input device " + id + " already exists");
		}
		hdl->input->devices[id] = dev;
	}

	extern void remove_input_device(msa::Handle hdl, const std::string &id)
	{
		if (hdl->input->devices.find(id) == hdl->input->devices.end())
		{
			throw std::logic_error("input device " + id + " does not exist");
		}
		InputDevice *dev = hdl->input->devices[id];
		if (dev->running)
		{
			// mark it as collectable by the calling thread
			dev->reap_in_runner = true;
			disable_input_device(hdl, id);
		}
		else
		{
			// no input_thread to take care of it, delete it ourselves
			dispose_input_device(dev);
		}
		hdl->input->devices.erase(id);
	}

	extern void get_input_devices(msa::Handle hdl, std::vector<std::string> *list)
	{
		std::map<std::string, InputDevice *> *devs = &hdl->input->devices;
		typedef std::map<std::string, InputDevice *>::iterator it_type;
		for (it_type iter = devs->begin(); iter != devs->end(); iter++)
		{
			// TODO: figure out why we cant have a vec of const str passed in.
			std::string id = iter->second->id;
			list->push_back(id);
		}
	}

	extern void enable_input_device(msa::Handle hdl, const std::string &id)
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
			throw std::logic_error("input device " + id + " does not exist");
		}
		// checks are done, we know it exists and is disabled
		InputDevice *dev = hdl->input->devices[id];
		InputThreadArgs *ita = new InputThreadArgs;
		ita->dev = dev;
		ita->hdl = hdl;

		msa::thread::Attributes attr;
		msa::thread::attr_init(&attr);
		msa::thread::attr_set_detach(&attr, true);
		dev->running = (msa::thread::create(&dev->thread, &attr, it_start, ita, "input") == 0);
		msa::thread::attr_destroy(&attr);
		
		if (dev->running)
		{
			hdl->input->active.push_back(dev->id);
			msa::log::info(hdl, "Enabled input device " + dev->id);
		}
		else
		{
			msa::log::warn(hdl, "Could not enable input device " + dev->id);
		}
	}

	extern void disable_input_device(msa::Handle hdl, const std::string &id)
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
			throw std::logic_error("input device " + id + " does not exist");
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
		if (INPUT_HANDLER_NAMES.empty())
		{
			INPUT_HANDLER_NAMES["get_tty_input"] = new InputHandler {get_tty_input, tty_ready};
		}
		return 0;
	}

	static int destroy_static_resources()
	{
		std::map<std::string, InputHandler *>::iterator it;
		for (it = INPUT_HANDLER_NAMES.begin(); it != INPUT_HANDLER_NAMES.end(); it++)
		{
			delete it->second;
		}
		INPUT_HANDLER_NAMES.clear();
		return 0;
	}

	static void read_config(msa::Handle hdl, const msa::config::Section &config)
	{
		if (config.has("TYPE") && config.has("ID") && config.has("HANDLER"))
		{
			// for each of the configs, read it in
			const std::vector<std::string> types = config.get_all("TYPE");
			const std::vector<std::string> ids = config.get_all("ID");
			const std::vector<std::string> handlers = config.get_all("HANDLER");
			for (size_t i = 0; i < types.size() && i < ids.size() && i < handlers.size(); i++)
			{
				std::string id = ids[i];
				std::string type_str = types[i];
				std::string handler_str = handlers[i];
				if (INPUT_TYPE_NAMES.find(type_str) == INPUT_TYPE_NAMES.end())
				{
					throw std::invalid_argument("'" + type_str + "' is not a valid input type");
				}
				if (INPUT_HANDLER_NAMES.find(handler_str) == INPUT_HANDLER_NAMES.end())
				{
					throw std::invalid_argument("'" + handler_str + "' is not a valid handler");
				}
				InputType type = INPUT_TYPE_NAMES[type_str];
				InputHandler *handler = INPUT_HANDLER_NAMES[handler_str];
				hdl->input->handlers[type] = handler;
				add_input_device(hdl, type, &id);
				enable_input_device(hdl, type_str + ":" + id);
			}
		}
	}

	static void create_input_device(InputDevice **dev_ptr, InputType type, const void *id)
	{
		InputDevice *dev = new InputDevice;
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
				dev->device_name = static_cast<const std::string *>(id);
				dev->id = "TTY:" + *dev->device_name;
				break;

			default:
				delete dev;
				throw std::invalid_argument("unknown input type: " + std::to_string(type));
				break;
		}
		*dev_ptr = dev;
	}

	static void dispose_input_device(InputDevice *dev)
	{	
		if (dev->running)
		{
			dev->running = false;
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
		typedef std::map<std::string, InputDevice *>::iterator it_type;
		it_type iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			InputDevice *dev = iter->second;
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
				dispose_input_device(iter->second);
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
		InputDevice *dev = ita->dev;
		delete ita;
		
		InputHandler *input_handler = it_get_handler(hdl, dev);

		msa::log::info(hdl, "Started reading from input device " + dev->id);
		it_read_input(hdl, dev, input_handler);
		msa::log::info(hdl, "Stopped reading from input device " + dev->id);

		it_cleanup(hdl, dev);
		
		return NULL;
	}

	static InputHandler *it_get_handler(msa::Handle hdl, InputDevice *dev)
	{
		if (hdl->input->handlers.find(dev->type) == hdl->input->handlers.end())
		{
			dev->running = false;
			disable_input_device(hdl, dev->id);
			throw std::logic_error("no handler for input device type " + std::to_string(dev->type));
		}
		return hdl->input->handlers[dev->type];
	}

	static void it_read_input(msa::Handle hdl, InputDevice *dev, InputHandler *input_handler)
	{
		while (dev->running)
		{
			if (input_handler->is_ready(hdl, dev))
			{
				InputChunk *chunk = input_handler->get_input(hdl, dev);
				msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, chunk);
			}
		}
	}

	static void it_cleanup(msa::Handle hdl, InputDevice *dev)
	{
		if (dev->reap_in_runner)
		{
			msa::log::info(hdl, "Freeing input device " + dev->id);
			dispose_input_device(dev);
		}
	}

	static InputChunk *get_tty_input(msa::Handle UNUSED(hdl), InputDevice *UNUSED(dev))
	{
		std::string input;
		std::cin >> input;
		InputChunk *ch = new InputChunk;
		ch->chars = input;
		return ch;
	}

	static bool tty_ready(msa::Handle UNUSED(hdl), InputDevice *UNUSED(dev))
	{
		return msa::util::check_stdin_ready();
	}

	static void interpret_cmd(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
	{
		InputChunk *ch = static_cast<InputChunk *>(e->args);
		std::string input = ch->chars;
		delete ch;
		if (input == "kill")
		{
			msa::event::generate(hdl, msa::event::Topic::COMMAND_EXIT, NULL);
		}
		else if (input == "announce")
		{
			msa::event::generate(hdl, msa::event::Topic::COMMAND_ANNOUNCE, NULL);
		}
		else
		{
			std::string * heap_alloc_str = new std::string(input);
			msa::event::generate(hdl, msa::event::Topic::INVALID_COMMAND, heap_alloc_str);
		}
	}

} }
