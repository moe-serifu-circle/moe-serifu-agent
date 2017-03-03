#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include "msa.hpp"
#include "configuration.hpp"

#include <string>
#include <vector>

namespace msa { namespace output {

	typedef enum { TTY, UDP, TCP } OutputType;

	typedef struct chunk_type Chunk;

	typedef struct device_type Device;

	typedef void (*OutputHandlerFunc)(msa::Handle hdl, const Chunk *ch, Device *dev);

	typedef struct output_handler_type OutputHandler;

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	
	extern void write(msa::Handle hdl, const Chunk *chunk);
	extern void add_device(msa::Handle hdl, OutputType type, void *device_id);
	extern void get_devices(msa::Handle hdl, std::vector<std::string> *list);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	extern void switch_device(msa::Handle hdl, const std::string &id);
	extern void get_active_device(msa::Handle hdl, std::string &id);
	
	extern void create_chunk(Chunk **chunk, const std::string &text);
	extern void dispose_chunk(Chunk *chunk);

	extern void create_handler(OutputHandler **handler, const std::string &name, OutputHandlerFunc func);
	extern void dispose_handler(OutputHandler *handler);
	extern const std::string &get_handler_name(const OutputHandler *handler);
	
	extern void register_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler);
	extern void unregister_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler);

} }

#endif
