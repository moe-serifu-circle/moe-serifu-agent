#include "input.hpp"
#include "event/dispatch.hpp"
#include "cxx_normalization.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <pthread.h>

namespace msa { namespace input {

	typedef InputChunk *(*InputHandler)(msa::Handle, InputDevice *);

	struct input_chunk_type
	{
		std::string chars;
	};

	struct input_device_type
	{
		std::string id;
		InputType type;
		pthread_t thread;
		bool running;
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
		std::map<InputType, InputHandler> handlers;
		std::map<std::string, InputHandler> handler_table;
	};

	typedef struct it_args_type
	{
		msa::Handle hdl;
		InputDevice *dev;
	} InputThreadArgs;

	static void create_input_device(InputDevice **dev, InputType type, const void *device_id);
	static void dispose_input_device(InputDevice *device);
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);

	static InputChunk *get_tty_input(msa::Handle hdl, InputDevice *dev);

	static void interpret_cmd(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	static void *it_start(void *hdl);

	extern int init(msa::Handle hdl)
	{
		int stat = create_input_context(&hdl->input);
		if (stat != 0)
		{
			return 1;
		}
		hdl->input->handler_table["get_tty_input"] = get_tty_input;
/*
		hdl->input->handlers[InputType::TTY] = get_tty_input;
		std::string id = "stdin";
		add_input_device(hdl, InputType::TTY, &id);
		enable_input_device(hdl, "TTY:stdin");
*//
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
		std::vector<std::string> &act = hdl->input->active;
		if (std::find(act.begin(), act.end(), id) != act.end())
		{
			disable_input_device(hdl, id);
		}
		InputDevice *dev = hdl->input->devices[id];
		hdl->input->devices.erase(id);
		dispose_input_device(dev);
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
		dev->running = (pthread_create(&dev->thread, NULL, it_start, ita) == 0);
		if (dev->running)
		{
			hdl->input->active.push_back(dev->id);
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
	}

	static void create_input_device(InputDevice **dev_ptr, InputType type, const void *id)
	{
		InputDevice *dev = new InputDevice;
		dev->running = false;
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
			dispose_input_device(iter->second);
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
		if (hdl->input->handlers.find(dev->type) == hdl->input->handlers.end())
		{
			dev->running = false;
			disable_input_device(hdl, dev->id);
			throw std::logic_error("no handler for input device type " + std::to_string(dev->type));
		}
		InputHandler input_handler = hdl->input->handlers[dev->type];
		while (dev->running)
		{
			InputChunk *chunk = input_handler(hdl, dev);
			msa::event::generate(hdl, msa::event::Topic::TEXT_INPUT, chunk);
		}
		return NULL;
	}

	static InputChunk *get_tty_input(msa::Handle UNUSED(hdl), InputDevice *UNUSED(dev))
	{
		std::string input;
		std::cin >> input;
		InputChunk *ch = new InputChunk;
		ch->chars = input;
		return ch;
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
