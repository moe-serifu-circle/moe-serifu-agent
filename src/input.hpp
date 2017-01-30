#ifndef INPUT_HPP
#define INPUT_HPP

namespace msa { namespace io {

	typedef enum input_type_type { TTY, TCP, UDP } InputType;

	typedef struct input_chunk_type InputChunk;

	typedef struct input_device_type InputDevice;

	typedef struct input_context_type InputContext;

	extern void create_input_context(InputContext **ctx);
	extern void dispose_input_context(InputContext *ctx);
	extern void create_input_device(InputContext *ctx, InputType type, void *device_id);
	extern void dispose_input_device(InputContext *ctx, const std::string &id);
} }

#endif
