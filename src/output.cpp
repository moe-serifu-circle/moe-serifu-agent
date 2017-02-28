#include "output.hpp"

#include <map>

namespace msa { namespace output {

	typedef void (*OutputHandler)(Chunk *ch, Device *dev);

	struct chunk_type
	{
		std::string *text;
	};

	struct device_type
	{
		std::string id;
		OutputType type;
		union
		{
			uint16_t port;
			const std::string device_name;
		};
	};

	struct output_context_type
	{
		std::map<std::string, Device *> devices;
		std::string active;
	};

	static int create_output_context(OutputContext **ctx);
	static int dispose_output_context(OutputContext *ctx);
	static int create_device(
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
		if (!dispose_output_context(&hdl->output))
		{
			return -1;
		}
		return 0;
	}
	
	extern void write(msa::Handle hdl, const Chunk *chunk);

	extern void write_text(msa::Handle hdl, const std::string &text)
	{
		Chunk *ch;
		create_chunk(&ch, text);
		write(hdl, ch);
		dispose_chunk(ch);
	}

	extern void add_device(msa::Handle hdl, OutputType type, void *device_id)
	{
		
	}
	
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	extern void switch_to_device(msa::Handle hdl, const std::string &id);
	extern void get_current_device(msa::Handle hdl, std::string &id);
	
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
		*ctx = output;
	}

} }
