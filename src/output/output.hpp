#ifndef MSA_OUTPUT_OUTPUT_HPP
#define MSA_OUTPUT_OUTPUT_HPP

#include "msa.hpp"
#include "cfg/cfg.hpp"

#include <string>
#include <vector>

namespace msa { namespace output {

	typedef enum { TTY, TCP, UDP } OutputType;

	typedef struct chunk_type Chunk;

	typedef struct device_type Device;

	typedef void (*OutputHandlerFunc)(msa::Handle hdl, const Chunk *ch, Device *dev);

	typedef struct output_handler_type OutputHandler;

	extern int init(msa::Handle hdl, const msa::cfg::Section &config);
	extern int quit(msa::Handle hdl);
	
	
	extern void add_device(msa::Handle hdl, OutputType type, void *device_id);
	extern void remove_device(msa::Handle hdl, const std::string &id);
	
	extern void register_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler);
	extern void unregister_handler(msa::Handle hdl, OutputType type, const OutputHandler *handler);
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "output/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "output/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
	
} }

#endif
