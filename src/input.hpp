#ifndef INPUT_HPP
#define INPUT_HPP

#include "environment.hpp"

namespace msa { namespace io {

	typedef enum input_type_type { TTY, TCP, UDP } InputType;

	typedef struct input_chunk_type InputChunk;

	typedef struct input_device_type InputDevice;

	extern int init(msa::core::Handle hdl);
	extern int quit(msa::core::Handle hdl);
	extern void create_input_device(msa::core::Handle hdl, InputType type, void *device_id);
	extern void dispose_input_device(msa::core::Handle hdl, const std::string &id);
} }

#endif
