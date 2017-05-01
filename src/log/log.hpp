#ifndef MSA_LOG_LOG_HPP
#define MSA_LOG_LOG_HPP

#include "msa.hpp"
#include "cfg/cfg.hpp"

#include <cstdint>
#include <string>

namespace msa { namespace log {

	enum class StreamType
	{
		FILE
	};

	// we have a constant named "DEBUG"; turn off that macro if it's on
	#ifdef DEBUG
		#define DEBUG_TEMP_OFF
		#undef DEBUG
	#endif
	enum class Level
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR
	};
	#ifdef DEBUG_TEMP_OFF
		#undef DEBUG_TEMP_OFF
		#define DEBUG
	#endif

	enum class Format
	{
		TEXT,
		XML
	};
	
	enum class OpenMode
	{
		OVERWRITE,
		APPEND
	};
	
	typedef size_t stream_id;

	extern int init(msa::Handle hdl, const msa::cfg::Section &config);
	extern int quit(msa::Handle hdl);

	// creates a new log stream and returns the ID of the stream
	extern stream_id create_stream(msa::Handle hdl, StreamType type, const std::string &location, Format fmt, const std::string &output_format_string, OpenMode open_mode);
	extern void set_level(msa::Handle hdl, Level level);
	extern void set_stream_level(msa::Handle hdl, stream_id id, Level level);
	extern Level get_stream_level(msa::Handle hdl, stream_id id);
	extern void trace(msa::Handle hdl, const char *msg);
	extern void debug(msa::Handle hdl, const char *msg);
	extern void info(msa::Handle hdl, const char *msg);
	extern void warn(msa::Handle hdl, const char *msg);
	extern void error(msa::Handle hdl, const char *msg);
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "log/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "log/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
} }

#endif
