#ifndef INPUT_HPP
#define INPUT_HPP

#include "msa.hpp"

#include <vector>
#include <string>

namespace msa { namespace input {

	typedef enum input_type_type { TTY, TCP, UDP } InputType;

	typedef struct input_chunk_type InputChunk;

	typedef struct input_device_type InputDevice;

	extern int init(msa::Handle hdl);
	extern int quit(msa::Handle hdl);
	extern void add_input_device(msa::Handle hdl, InputType type, void *device_id);
	extern void get_input_devices(msa::Handle hdl, std::vector<const std::string> *list);
	extern void remove_input_device(msa::Handle hdl, const std::string &id);
	extern void enable_input_device(msa::Handle hdl, const std::string &id);
	extern void disable_input_device(msa::Handle hdl, const std::string &id);
} }

#endif
