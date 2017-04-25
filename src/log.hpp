#ifndef MSA_LOG_HPP
#define MSA_LOG_HPP

#include "msa.hpp"
#include "cfg/cfg.hpp"

#include <cstdint>
#include <string>

namespace msa { namespace log {

	typedef enum stream_type_type { FILE } StreamType;

	// we have a constant named "DEBUG"; turn off that macro if it's on
	#ifdef DEBUG
		#define DEBUG_TEMP_OFF
		#undef DEBUG
	#endif
	typedef enum level_type { TRACE, DEBUG, INFO, WARN, ERROR } Level;
	#ifdef DEBUG_TEMP_OFF
		#undef DEBUG_TEMP_OFF
		#define DEBUG
	#endif

	typedef enum format_type { TEXT, XML } Format;
	typedef enum open_mode_type { OVERWRITE, APPEND } OpenMode;
	typedef size_t stream_id;

	extern int init(msa::Handle hdl, const msa::config::Section &config);

	extern int quit(msa::Handle hdl);

	// creates a new log stream and returns the ID of the stream
	extern stream_id create_stream(msa::Handle hdl, StreamType type, const std::string &location, Format fmt, const std::string &output_format_string, OpenMode open_mode);
	extern void set_level(msa::Handle hdl, Level level);
	extern Level get_level(msa::Handle hdl);
	extern void set_stream_level(msa::Handle hdl, stream_id id, Level level);
	extern Level get_stream_level(msa::Handle hdl, stream_id id);

	extern void trace(msa::Handle hdl, const std::string &msg);
	extern void trace(msa::Handle hdl, const char *msg);
	extern void debug(msa::Handle hdl, const std::string &msg);
	extern void debug(msa::Handle hdl, const char *msg);
	extern void info(msa::Handle hdl, const std::string &msg);
	extern void info(msa::Handle hdl, const char *msg);
	extern void warn(msa::Handle hdl, const std::string &msg);
	extern void warn(msa::Handle hdl, const char *msg);
	extern void error(msa::Handle hdl, const std::string &msg);
	extern void error(msa::Handle hdl, const char *msg);
} }

#endif
