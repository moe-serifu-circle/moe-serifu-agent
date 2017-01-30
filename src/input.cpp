#include "input.hpp"
#include "control.hpp"

#include <map>
#include <string>

namespace msa { namespace io {

	struct input_chunk_type
	{
		std::string chars;
	};

	struct input_device_type
	{
		std::string id;
		InputType type;
		union
		{
			uint16_t port;
			std::string device_name;
		}
	};	

	struct input_context_type
	{
		std::map<std::string, const InputDevice *> devices;
		InputDevice *current_device;
	};

	extern void create_input_context(InputContext **ctx)
	{
		InputContext *io_ctx = new InputContext;
		io_ctx->current_device = NULL;
		*ctx = io_ctx;
	}

	extern void dispose_input_context(InputContext *ctx)
	{
		typedef std::map<std::string, const InputDevice *>::iterator it_type;
		it_type iter = ctx->devices.begin();
		while (iter != ctx->devices.end())
		{
			dispose_input_device(iter->second);
			iter = ctx->devices.erase(iter);
		}
		delete ctx;
	}

	extern void create_input_device(InputContext *ctx, InputType type, void *id)
	{
		InputDevice *dev = new InputDevice;
		dev->type = type;
		switch (dev->type)
		{
			case InputType::TTY:
				dev->device_name = 
		}
	}

} }
