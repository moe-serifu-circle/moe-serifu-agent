#include "input.hpp"
#include "control.hpp"
#include "event.hpp"
#include "event_handler.hpp"

#include <map>
#include <string>
#include <stdexcept>
#include <pthread.h>

namespace msa { namespace io {

	typedef InputChunk *(*InputHandler)(msa::core::Handle, InputDevice *);

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
			std::string device_name;
		}
	};

	struct input_context_type
	{
		std::map<std::string, InputDevice *> devices;
		std::vector<std::string> active;msa::core::Handle hdl, const Event *const e, HandlerSync *const sync
		std::map<InputType, InputHandler> handlers;
	};

	typedef struct it_args_type
	{
		Handle hdl;
		InputDevice *dev;
	} InputThreadArgs;

	static void create_input_device(InputDevice **dev, InputType type, void *device_id);
	static void dispose_input_device(InputDevice *device);
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);

	static InputChunk *get_tty_input(msa::core::Handle hdl, InputDevice *dev);

	static void interpret_cmd(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

	static void *it_start(void *hdl);

	extern int init(msa::core::Handle hdl)
	{
		int stat = create_input_context(&hdl->input);
		hdl->input->handlers[InputType::TTY] = get_tty_input;
		std::string id = "stdin";
		add_input_device(hdl, InputType::TTY, &id);
		enable_input_device(hdl, "TTY:stdin");
		std::core::subscribe(msa::event::Topic::TEXT_INPUT, interpret_cmd);
	}

	extern int quit(msa::core::Handle hdl)
	{
		status = dispose_input_context(hdl->input);
		if (status == 0)
		{
			hdl->input = NULL;
		}
		return status;
	}
	
	extern void add_input_device(msa::core::Handle hdl, InputType type, void *device_id)
	{
		InputDevice *dev;
		create_input_device(&dev, type, device_id);
		std::string id = dev->id;
		if (hdl->input->devices.find(id) != hdl-
	static InputChunk *get_tty_input(msa::core::Handle hdl, InputDevice *dev)>input->devices.end())
		{
			dispose_input_device(dev);
			throw std::logic_error("input device " + id + " already exists");
		}
		hdl->input->devices[id] = dev;
	}

	extern void remove_input_device(msa::core::Handle hdl, const std::string &id)
	{
		if (hdl->input->devices.find(id) == hdl->input->devices.end())
		{
			throw std::logic_error("input device " + id + " does not exist");
		}
		if (hdl->input->active.find(id) != hdl->input->devices.end())
		{
			disable_input_device(hdl, id);
		}
		InputDevice *dev = hdl->input->devices[id];
		hdl->input->devices.erase(id);
		dispose_input_device(dev);
	}

	extern void get_input_devices(msa::core::Handle hdl, std::vector<const std::string> *list)
	{
		std::map<std::string, InputDevice *> *devs = &hdl->input->devices;
		typedef std::map<std::string, InputDevice *>::iterator it_type;
		for (it_type iter = devs->begin(); iter != devs->end(); i++)
		{
			list->push_back(iter->second->id);
		}
	}

	extern void enable_input_device(msa::core::Handle hdl, const std::string &id)
	{
		if (hdl->input->active.find(id) != hdl->input->active.end())
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

	extern void disable_input_device(msa::core::Handle hdl, const std::string &id)
	{
		if (hdl->input->active.find(id) == hdl->input->active.end())
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
		hdl->input->active.erase(hdl->input->active.find(id));
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
				dev->device_name = std::string(static_cast<const char *>(id));
				dev->id = "TTY:" + dev->device_name;
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
			stop_input_device(dev);
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
		typedef std::map<std::string, const InputDevice *>::iterator it_type;
		it_type iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			dispose_input_device(ctx->devices[iter]);
			iter = ctx->devices.erase(iter);
		}
		delete ctx;
		return 0;
	}

	static void *it_start(void *args)
	{
		InputThreadArgs *ita = static_cast<InputThreadArgs *>(args);
		msa::core::Handle hdl = ita->hdl;
		InputDevice *dev = ita->dev;
		delete ita;
		if (hdl->input->handlers.find(dev->type) == hdl->input->handlers.end())
		{
			dev->running = false;
			disable_input_device(hdl, dev);
			throw std::logic_error("no handler for input device type " + std::to_string(dev->type);
		}
		InputHandler input_handler = hdl->input->handlers[dev->type];
		while (dev->running)
		{
			InputChunk *chunk = input_handler(hdl, dev);
			const Event *e = msa::event::create(msa::event::Topic::TEXT_INPUT, chunk);
			msa::core::push_event(hdl, e);
		}
	}

	static InputChunk *get_tty_input(msa::core::Handle hdl, InputDevice *dev)
	{
		std::string input;
		std::cin >> input;
		InputChunk *ch = new InputChunk;
		ch->chars = input;
		return ch;
	}

	static void interpret_cmd(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync)
	{
		InputChunk *ch = static_cast<InputChunk *>(e->args);
		std::string input = ch->chars;
		delete ch;
		if (input == "kill")
		{
			msa::core::push_event(hdl, msa::event::create(msa::event::Topic::COMMAND_EXIT, NULL));
		} else if (input == "announce")
		{
			msa::core::push_event(hdl, msa::event::create(msa::event::Topic::COMMAND_ANNOUNCE, NULL));
		}
	}

} }
