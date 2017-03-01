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

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	
	extern void write(msa::Handle hdl, const Chunk *chunk);
	extern void add_device(msa::Handle hdl, InputType type, void *device_id);
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	extern void switch_device(msa::Handle hdl, const std::string &id);
	extern void get_active_device(msa::Handle hdl, std::string &id);
	
	extern void create_chunk(Chunk **chunk, const std::string &text);
	extern void dispose_chunk(Chunk *chunk);

} }

#endif
