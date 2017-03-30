#ifndef MSA_INPUT_HPP
#define MSA_INPUT_HPP

#include "msa.hpp"
#include "configuration.hpp"

#include <vector>
#include <string>

namespace msa { namespace input {

	typedef enum input_type_type { TTY, TCP, UDP } InputType;

	typedef struct chunk_type
	{
		std::string text;
	} Chunk;

	typedef struct device_type Device;

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	extern int quit(msa::Handle hdl);
	extern void add_device(msa::Handle hdl, InputType type, void *device_id);
	extern void get_devices(msa::Handle hdl, std::vector<const std::string> *list);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	extern void enable_device(msa::Handle hdl, const std::string &id);
	extern void disable_device(msa::Handle hdl, const std::string &id);
} }

#endif
