#include "input.hpp"
#include "control.hpp"

#include <map>
#include <vector>
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
		std::vector<std::string> active;
	};

	extern void enable_input_device(msa::core::Handle hdl, const std::string &id);
	extern void disable_input_device(msa::core::Handle hdl, const std::string &id);

	static void create_input_device(InputDevice **dev, InputType type, void *device_id);
	static void dispose_input_device(InputDevice *device);
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);

	static void *it_start(void *hdl);

	extern int init(msa::core::Handle hdl)
	{
		return create_input_context(&hdl->input);
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
		if (hdl->input->devices.find(id) != hdl->input->devices.end())
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
		std::map<std::string, InputDevice *>
		typedef std::map<std::string, const InputDevice *>::iterator it_type;
		for (it_type = hdl->input->devices.begin(); i < hdl->input->devices.size(); i++)
		{
			list->push_back(hdl->input->devices[i]->
		}
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
		msa::core::Handle hdl = static_cast<msa::core::Handle>(args);
	}

} }
