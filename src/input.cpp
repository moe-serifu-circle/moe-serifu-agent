#include "input.hpp"
#include "control.hpp"

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
		InputDevice *current_device;
	};
	
	static int create_input_context(InputContext **ctx);
	static int dispose_input_context(InputContext *ctx);
	static void *it_start(void *hdl);

	static inline void assert_exists(msa::core::Handle hdl, const std::string &id);

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

	extern void select_input_channel(msa::core::Handle hdl, const std::string &id)
	{
		InputContext *ctx = hdl->input;
		if (ctx->current_device->id == id)
		{
			// we're done, device is already selected.
			return;
		}
		assert_exists(hdl, id, true);
		if (ctx->current_device != NULL)
		{
			// bring it down
		}
		ctx->current_device = ctx->devices[id];
		
	}

	extern void start_input_channel(msa::core::Handle hdl, const std::string &id)
	{
		InputContext *ctx = hdl->input;
		if (ctx->current_device != NULL)
		{
			throw std::logic_error("cannot start input channel while another is running");
		}
		assert_exists(hdl, id, true);
		InputDevice *dev = ctx->devices[id];
		ctx->current_device = dev;
		dev->running = pthread_create(&dev->thread, NULL, it_start, hdl);
	}

	extern void stop_input_channel(msa::core::Handle hdl, const std::string &id)
	{
		if (hdl->input->current_device == NULL)
		{
			return;
		}
		if (hdl->input->current_device->id != id)
		{
			throw std::logic_error("current input channel is not " + id);
		}
		if (hdl->input->current_device->running)
		{
			hdl->input->current_device->running = false;
		}
		hdl->input->current_device = NULL;
	}

	extern void create_input_device(msa::core::Handle hdl, InputType type, const void *id)
	{
		InputContext *ctx = hdl->input;
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
		assert_exists(hdl, dev->id, false);
		ctx->devices[dev->id] = dev;
	}

	extern void dispose_input_device(msa::core::Handle hdl, const std::string &id)
	{
		InputContext *ctx = hdl->input;
		assert_exists(hdl, id, true);
		InputDevice *dev = ctx->devices[id];
		if (ctx->current_device == dev)
		{
			ctx->current_device = NULL;
		}
		delete dev;
		ctx->devices.erase(id);
	}

	static int create_input_context(InputContext **ctx)
	{
		InputContext *io_ctx = new InputContext;
		io_ctx->current_device = NULL;
		*ctx = io_ctx;
		return 0;
	}

	static int dispose_input_context(InputContext *ctx)
	{
		typedef std::map<std::string, const InputDevice *>::iterator it_type;
		it_type iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			dispose_input_device(iter->second);
			iter = ctx->devices.erase(iter);
		}
		delete ctx;
		return 0;
	}

	static inline void assert_exists(msa::core::Handle hdl, const std::string &id, bool expected)
	{
		InputContext *ctx = hdl->input;
		bool exists = (ctx->devices.find(id) != ctx->devices.end())
		if (exists != expected)
		{
			std::string err = (expected ? "does not exist" : "already exists");
			throw std::invalid_argument("device " + id + " " + err);
		}
	}

	static void *it_start(void *args)
	{
		msa::core::Handle hdl = static_cast<msa::core::Handle>(args);
	}

} }
