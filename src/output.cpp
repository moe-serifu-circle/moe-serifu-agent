#include "output.hpp"
#include "log.hpp"

#include <map>
#include <stdexcept>

namespace msa { namespace output {

	typedef void (*OutputHandlerFunc)(Chunk *ch, Device *dev);

	struct chunk_type
	{
		std::string *text;
	};

	struct device_type
	{
		std::string id;
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
		Mutex *usage_mutex;
		std::map<std::string, Device *> devices;
		std::string active;
		bool running;
	};
	
	std::map<OutputType, std::map<std::string, OutputHandlerFunc>> handlers;

	static int create_output_context(OutputContext **ctx);
	static int dispose_output_context(OutputContext *ctx);
	static int create_device(Device **dev);
	static int dispose_device(Device *dev);
	static void read_config(msa::Handle hdl, msa::config::Section &config);

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
		msa::thread::mutex_lock(ctx->usage_mutex);
		
		msa::thread::mutex_unlock(ctx->usage_mutex);
	}

	extern void write_text(msa::Handle hdl, const std::string &text)
	{
		Chunk *ch;
		create_chunk(&ch, text);
		write(hdl, ch);
		dispose_chunk(ch);
	}

	extern void add_device(msa::Handle hdl, OutputType type, void *device_id)
	{
		OutputContex *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->usage_mutex);
		Device *dev;
		create_device(&dev, type, device_id);
		const std::string &id = dev->id;
		if (ctx->devices.find(id) != ctx->devices.end())
		{
			dispose_device(dev);
			msa::thread::mutex_unlock(ctx->usage_mutex);
			throw std::invalid_argument("output device already exists: " + std::to_string(dev->id));
		}
		ctx->devices[id] = dev;
		msa::thread::mutex_unlock(ctx->usage_mutex);
		msa::log::info("Added output device " + id);
	}
	
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list)
	{
		msa::thread::mutex_lock(hdl->output->usage_mutex);
		std::map<std::string, Device *> *devs = *hdl->output->devices;
		std::map<std::string, Device *>::const_iterator iter;
		for (iter = ctx->devices.begin(); iter != ctx->devices.end(); iter++)
		{
			std::string id = iter->second->id;
			list->push_back(id);
		}
		msa::thread::mutex_unlock(hdl->output->usage_mutex);
	}
	
	extern void remove_device(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->usage_mutex);
		if (ctx->devices.find(id) == ctx->devices.end())
		{
			msa::thread::mutex_unlock(ctx->usage_mutex);
			throw std::invalid_argument("output device does not exist: " + id);
		}
		if (ctx->active == id)
		{
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
				msa::thread::mutex_unlock(ctx->usage_mutex);
				throw std::logic_error("cannot remove sole output device " + id);
			}
			// otherwise, switch out before removing
			msa::thread::mutex_unlock(ctx->usage_mutex);
			switch_device(hdl, next_id);
			msa::thread::mutex_lock(ctx->usage_mutex);
		}
		dispose_device(ctx->devices[id]);
		ctx->devices.erase(id);
		msa::thread::mutex_unlock(ctx->usage_mutex);
		msa::log::info("Removed output device " + id);
	}
	
	extern void switch_device(msa::Handle hdl, const std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->usage_mutex);
		if (ctx->devices.find(id) == ctx->devices.end())
		{
			msa::thread::mutex_unlock(ctx->usage_mutex);
			throw std::invalid_argument("output device does not exist: " + id);
		}
		if (ctx->active != "")
		{
			ctx->devices[ctx->active]->active = false;
		}
		ctx->active = id;
		ctx->devices[id]->active = true;
		msa::thread::mutex_unlock(ctx->usage_mutex);
	}

	extern void get_active_device(msa::Handle hdl, std::string &id)
	{
		OutputContext *ctx = hdl->output;
		msa::thread::mutex_lock(ctx->usage_mutex);
		id = ctx->active;
		msa::thread::mutex_unlock(ctx->usage_mutex);
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

	static int create_output_context(OutputContext **ctx)
	{
		output = new OutputContext;
		msa::thread::mutex_init(output->usage_mutex, NULL);
		*ctx = output;
		return 0;
	}

	static int dispose_output_context(OutputContext *ctx)
	{
		Device *dev = ctx->devices[ctx->active];
		dev->active = false;
		dispose_device(dev);
		ctx->active = "";
		msa::thread::mutex_destroy(output->mod_mutex);
		delete ctx;
	}
	
	static int create_device(Device **dev_ptr, OutputType type, const void *id)
	{
		Device *dev = new Device;
		dev->type = type;
		dev->active = false;
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

} }
