#include "log.hpp"
#include "string.hpp"

#include <ofstream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>

namespace msa { namespace log {

	static std::map<std::string, Level> LEVEL_NAMES;
	static std::map<std::string, Format> FORMAT_NAMES;
	static std::map<std::string, StreamType> STREAM_TYPE_NAMES;

	typedef int (*CloseHandler)(std::ostream &raw_stream);

	typedef struct log_stream_type
	{
		std::ostream *out;
		StreamType type;
		CloseHandler close_handler;
		Level level;
		Format format;
	} LogStream;

	struct log_context_type
	{
		std::vector<LogStream *> streams;
		Level level;
	}

	static int create_log_context(LogContext **ctx);
	static int dispose_log_context(LogContext *ctx);
	static int create_log_stream(LogStream **stream);
	static int dispose_log_stream(LogStream *stream);
	static int close_ofstream(std::ostream &raw_stream);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		// init static resources
		if (LEVEL_NAMES.empty())
		{
			LEVEL_NAMES["TRACE"] = Level::TRACE;
			LEVEL_NAMES["DEBUG"] = Level::DEBUG;
			LEVEL_NAMES["INFO"] = Level::INFO;
			LEVEL_NAMES["WARNING"] = Level::WARNING;
			LEVEL_NAMES["ERROR"] = Level::ERROR;
		}
		if (FORMAT_NAMES.empty())
		{
			FORMAT_NAMES["TEXT"] = Format::TEXT;
			FORMAT_NAMES["XML"] = Format::XML;
		}
		if (STREAM_TYPE_NAMES.empty())
		{
			STREAM_TYPE_NAMES["FILE"] = StreamType::FILE;
		}

		create_log_context(hdl->log);

		// first check config for global level
		std::string gl_level_str = config.get_or("GLOBAL_LEVEL", "INFO");
		msa::util::to_upper(gl_level_str);
		if (LEVEL_NAMES.find(gl_level_str) == LEVEL_NAMES.end())
		{
			throw std::invalid_argument("'" + gl_level_str + "' is not a valid log level");
		}
		hdl->log->level = LEVEL_NAMES[gl_level_str];
		
		// now check config for individual streams
		if (config.has("TYPE") && config.has("LOCATION"))
		{
			const std::vector<std::string> types = config.get_all("TYPE");
			const std::vector<std::string> locs = config.get_all("LOCATION");
			const std::vector<std::string> levs = config.has("LEVEL") ? config.get_all("LEVEL") : std::vector<std::string>();
			const std::vector<std::string> fmts = config.has("FORMAT") ? config.get_all("FORMAT") : std::vector<std::string>();

			for (size_t i = 0; i < types.size() && i < locs.size())
			{
				std::string type_str = types[i];
				std::string location = locs[i];
				std::string lev_str = levs.size() > i ? levs[i] : "info";
				std::string fmt_str = fmts.size() > i ? fmts[i] : "xml";
				msa::util::to_upper(type_str);
				msa::util::to_upper(lev_str);
				msa::util::to_upper(fmt_str);
				if (FORMAT_NAMES.find(fmt_str) == FORMAT_NAMES.end())
				{
					throw std::invalid_argument("'" + fmt_str + "' is not a valid log format");
				}
				if (LEVEL_NAMES.find(lev_str) == LEVEL_NAMES.end())
				{
					throw std::invalid_argument("'" + lev_str + "' is not a valid log level");
				}
				if (STREAM_TYPE_NAMES.find(type_str) == STREAM_TYPE_NAMES.end())
				{
					throw std::invalid_argument("'" + type_str + "' is not a valid log stream type");
				}
				StreamType type = STREAM_TYPE_NAMES[type_str];
				Level lev = LEVEL_NAMES[lev_str];
				Format fmt = FORMAT_NAMES[fmt_str];
				stream_id id = create_stream(hdl, type, location, fmt);
				set_stream_level(hdl, id, lev);
			}
		}
	}

	extern int quit(msa::Handle hdl)
	{
		dispose_log_context(hdl->log);
		return 0;
	}

	extern stream_id create_stream(msa::Handle hdl, StreamType type, const std::string &location, Format fmt)
	{
		LogStream *s;
		if (create_log_stream(&s) != 0)
		{
			throw std::logic_error("could not create log stream");
		}
		s->type = type;
		s->format = fmt;
		
		// now actually open the stream
		if (s->type == StreamType::FILE)
		{
			std::ofstream;
			
		}
		else
		{
			dispose_log_stream(s);
			throw std::invalid_argument("unknown log stream type: " + std::to_string(s->type);
		}

		hdl->log->streams.push_back(*s);
		return (stream_id) (hdl->log->streams.size() - 1);
	}

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

	static int create_log_context(LogContext **ctx)
	{
		LogContext *log = new LogContext;
		*ctx = log;
		return 0;
	}

	static int dispose_log_context(LogContext *ctx)
	{
		while (!ctx->streams.empty())
		{
			LogStream *stream = *(ctx->streams.begin());
			if (dispose_log_stream(stream) != 0)
			{
				return 1;
			}
			ctx->streams.erase(ctx->streams.begin());
		}
		delete ctx;
		return 0;
	}

	static int create_log_stream(LogStream **stream)
	{
		LogStream *st = new LogStream;
		st->out = NULL;
		st->level = Level::TRACE;
		st->format = Format::TEXT;
		st->close_handler = NULL;
		*stream = st;
		return 0;
	}

	static int dispose_log_stream(LogStream *stream)
	{
		int stat = stream->close_handler(stream->out);
		if (stat != 0)
		{
			return stat;
		}
		delete stream;
		return 0;
	}

	static int close_ofstream(std::ostream &raw_stream)
	{
		std::ofstream file = static_cast<std::ofstream>(raw_stream);
		file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		try {
			if (file.is_open())
			{
				file.close();
			}
		}
		catch (...)
		{
			return 1;
		}
		return 0;
	}

} }
