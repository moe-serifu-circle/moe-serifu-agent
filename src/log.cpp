#include "log.hpp"
#include "string.hpp"
#include "util.hpp"

#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <queue>

#include <ctime>
#include <cstdio>
#include <cstring>

#include "platform/thread/thread.hpp"

namespace msa { namespace log {

	static std::map<std::string, Level> LEVEL_NAMES;
	static std::map<std::string, Format> FORMAT_NAMES;
	static std::map<std::string, OpenMode> OPEN_MODE_NAMES;
	static std::map<std::string, StreamType> STREAM_TYPE_NAMES;
	static std::string XML_FORMAT_STRING = "<entry><time>%1$s</time><thread>%2$s</thread><level>%3$s</level><message>%4$s</message></entry>";

	typedef int (*CloseHandler)(std::ostream *raw_stream);

	typedef struct message_type
	{
		const std::string *text;
		Level level;
		time_t time;
		const std::string *thread;
	} Message;

	typedef struct log_stream_type
	{
		std::ostream *out;
		std::string output_format_string;
		StreamType type;
		CloseHandler close_handler;
		Level level;
		Format format;
		OpenMode open_mode;
	} LogStream;

	struct log_context_type
	{
		std::vector<LogStream *> streams;
		Level level;
		msa::thread::Thread writer_thread;
		msa::thread::Mutex queue_mutex;
		std::queue<Message *> messages;
		bool running;
	};

	static int init_static_resources();
	static void read_config(msa::Handle hdl, const msa::config::Section &config);
	static int create_log_context(LogContext **ctx);
	static int dispose_log_context(LogContext *ctx);
	static int create_log_stream(LogStream **stream);
	static int dispose_log_stream(LogStream *stream);
	static int create_message(Message **msg, const std::string &msg_text, Level level);
	static int dispose_message(Message *msg);
	static int close_ofstream(std::ostream *raw_stream);
	static void check_and_push(msa::Handle hdl, const std::string &msg_text, Level level);
	static void push_msg(msa::Handle hdl, Message *msg);
	static const char *level_to_str(Level lev);
	static const char *get_time_str(const struct tm *time_info);
	
	static void *writer_start(void *args);
	static Message *writer_poll_msg(msa::Handle hdl);
	static void writer_write_to_streams(msa::Handle hdl, const Message *msg);
	static void writer_write(LogStream *stream, const Message *ctx);
	static void writer_write_xml(LogStream *stream, const Message *ctx);
	static void writer_write_text(LogStream *stream, const Message *ctx);

	extern int init(msa::Handle hdl, const msa::config::Section &config)
	{
		init_static_resources();
		create_log_context(&hdl->log);
		read_config(hdl, config);
		hdl->log->running = true;
		// spawn the thread
		int status = msa::thread::create(&hdl->log->writer_thread, NULL, writer_start, hdl, "log-writer");
		if (status != 0)
		{
			hdl->log->running = false;
			dispose_log_context(hdl->log);
			return 1;
		}

		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		hdl->log->running = false;
		msa::thread::join(hdl->log->writer_thread, NULL);
		dispose_log_context(hdl->log);
		return 0;
	}

	extern stream_id create_stream(msa::Handle hdl, StreamType type, const std::string &location, Format fmt, const std::string &output_format_string, OpenMode open_mode)
	{
		LogStream *s;
		if (create_log_stream(&s) != 0)
		{
			throw std::logic_error("could not create log stream");
		}
		s->type = type;
		s->format = fmt;
		s->output_format_string = output_format_string;
		// now actually open the stream
		if (s->type == StreamType::FILE)
		{
			std::ofstream *file = new std::ofstream;
			file->exceptions(std::ofstream::failbit | std::ofstream::badbit);
			std::ofstream::openmode mode = std::ofstream::out;
			mode |= (open_mode == OpenMode::APPEND) ? std::ofstream::app : std::ofstream::trunc;
			file->open(location, mode);
			if (!file->is_open())
			{
				throw std::logic_error("could not open log stream file");
			}
			s->out = file;
			s->close_handler = close_ofstream;
		}
		else
		{
			dispose_log_stream(s);
			throw std::invalid_argument("unknown log stream type: " + std::to_string(s->type));
		}

		hdl->log->streams.push_back(s);
		return (stream_id) (hdl->log->streams.size() - 1);
	}

	extern void set_level(msa::Handle hdl, Level level)
	{
		hdl->log->level = level;
	}

	extern Level get_level(msa::Handle hdl)
	{
		return hdl->log->level;
	}

	extern void set_stream_level(msa::Handle hdl, stream_id id, Level level)
	{
		hdl->log->streams.at(id)->level = level;
	}
	
	extern Level get_stream_level(msa::Handle hdl, stream_id id)
	{
		return hdl->log->streams.at(id)->level;
	}

	extern void trace(msa::Handle hdl, const std::string &msg)
	{
		check_and_push(hdl, msg, Level::TRACE);
	}

	extern void trace(msa::Handle hdl, const char *msg)
	{
		std::string msg_str = std::string(msg);
		trace(hdl, msg_str);
	}

	extern void debug(msa::Handle hdl, const std::string &msg)
	{
		check_and_push(hdl, msg, Level::DEBUG);
	}

	extern void debug(msa::Handle hdl, const char *msg)
	{
		std::string msg_str = std::string(msg);
		debug(hdl, msg_str);
	}

	extern void info(msa::Handle hdl, const std::string &msg)
	{
		check_and_push(hdl, msg, Level::INFO);
	}

	extern void info(msa::Handle hdl, const char *msg)
	{
		std::string msg_str = std::string(msg);
		info(hdl, msg_str);
	}

	extern void warn(msa::Handle hdl, const std::string &msg)
	{
		check_and_push(hdl, msg, Level::WARN);
	}

	extern void warn(msa::Handle hdl, const char *msg)
	{
		std::string msg_str = std::string(msg);
		warn(hdl, msg_str);
	}

	extern void error(msa::Handle hdl, const std::string &msg)
	{
		check_and_push(hdl, msg, Level::ERROR);
	}

	extern void error(msa::Handle hdl, const char *msg)
	{
		std::string msg_str = std::string(msg);
		error(hdl, msg_str);
	}

	static int init_static_resources()
	{
		if (LEVEL_NAMES.empty())
		{
			LEVEL_NAMES["TRACE"] = Level::TRACE;
			LEVEL_NAMES["DEBUG"] = Level::DEBUG;
			LEVEL_NAMES["INFO"] = Level::INFO;
			LEVEL_NAMES["WARN"] = Level::WARN;
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
		if (OPEN_MODE_NAMES.empty())
		{
			OPEN_MODE_NAMES["OVERWRITE"] = OpenMode::OVERWRITE;
			OPEN_MODE_NAMES["APPEND"] = OpenMode::APPEND;
		}
		return 0;
	}
	
	static void read_config(msa::Handle hdl, const msa::config::Section &config)
	{
		// first check for global level
		std::string gl_level_str = config.get_or("GLOBAL_LEVEL", "INFO");
		msa::string::to_upper(gl_level_str);
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
			const std::vector<std::string> outputs = config.has("OUTPUT") ? config.get_all("OUTPUT") : std::vector<std::string>();
			const std::vector<std::string> open_modes = config.has("OPEN_MODE") ? config.get_all("OPEN_MODE") : std::vector<std::string>();

			for (size_t i = 0; i < types.size() && i < locs.size(); i++)
			{
				std::string type_str = types[i];
				std::string location = locs[i];
				std::string lev_str = levs.size() > i ? levs[i] : "info";
				std::string fmt_str = fmts.size() > i ? fmts[i] : "xml";
				std::string open_mode_str = open_modes.size() > i ? open_modes[i] : "append";
				
				msa::string::to_upper(type_str);
				msa::string::to_upper(lev_str);
				msa::string::to_upper(fmt_str);
				msa::string::to_upper(open_mode_str);
				
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
				if (OPEN_MODE_NAMES.find(open_mode_str) == OPEN_MODE_NAMES.end())
				{
					throw std::invalid_argument("'" + open_mode_str + "' is not a valid log open mode");
				}

				StreamType type = STREAM_TYPE_NAMES[type_str];
				Level lev = LEVEL_NAMES[lev_str];
				Format fmt = FORMAT_NAMES[fmt_str];
				OpenMode open_mode = OPEN_MODE_NAMES[open_mode_str];

				std::string output;
				if (fmt == Format::TEXT)
				{
					if (outputs.size() <= i) {
						throw std::invalid_argument("TEXT log format requires OUTPUT parameter");
					}
					output = outputs.at(i);
				}
				else if (fmt == Format::XML)
				{
					output = XML_FORMAT_STRING;
				}
				
				stream_id id = create_stream(hdl, type, location, fmt, output, open_mode);
				set_stream_level(hdl, id, lev);
			}
		}
	}

	static void check_and_push(msa::Handle hdl, const std::string &msg_text, Level level)
	{
		LogContext *ctx = hdl->log;
		// check if we should ignore the message from the global level
		if (level < ctx->level) {
			return;
		}
		
		Message *msg;
		if (create_message(&msg, msg_text, level) != 0)
		{
			throw std::runtime_error("could not create log message");
		}
		push_msg(hdl, msg);
	}

	static const char *level_to_str(Level lev)
	{
		if (lev == Level::TRACE)
		{
			return "TRACE";
		}
		else if (lev == Level::DEBUG)
		{
			return "DEBUG";
		}
		else if (lev == Level::INFO)
		{
			return "INFO";
		}
		else if (lev == Level::WARN)
		{
			return "WARN";
		}
		else if (lev == Level::ERROR)
		{
			return "ERROR";
		}
		else
		{
			return "UNKNOWN";
		}
	}

	static int create_log_context(LogContext **ctx)
	{
		LogContext *log = new LogContext;
		log->running = false;
		msa::thread::mutex_init(&log->queue_mutex, NULL);
		*ctx = log;
		return 0;
	}

	static int dispose_log_context(LogContext *ctx)
	{
		msa::thread::mutex_destroy(&ctx->queue_mutex);
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
		if (stream->out != NULL)
		{
			int stat = stream->close_handler(stream->out);
			if (stat != 0)
			{
				return stat;
			}
			delete stream->out;
		}
		delete stream;
		return 0;
	}

	static int create_message(Message **msg, const std::string &msg_text, Level level)
	{
		Message *m = new Message;
		time(&m->time);
		m->text = new std::string(msg_text);
		m->level = level;

		// get the calling thread's name
		char buf[16];
		if (msa::thread::get_name(msa::thread::self(), buf, 16) != 0)
		{
			delete m->text;
			delete m;
			return 1;
		}
		m->thread = new std::string(buf);

		*msg = m;
		return 0;
	}

	static int dispose_message(Message *msg)
	{
		delete msg->thread;
		delete msg->text;
		delete msg;
		return 0;
	}

	static int close_ofstream(std::ostream *raw_stream)
	{
		std::ofstream *file = static_cast<std::ofstream *>(raw_stream);
		try {
			if (file->is_open())
			{
				file->close();
			}
		}
		catch (...)
		{
			return 1;
		}
		return 0;
	}

	static void push_msg(msa::Handle hdl, Message *msg)
	{
		// do not allow any messages after shutdown
		if (!hdl->log->running)
		{
			throw std::logic_error("cannot write to log when log module is not running");
		}
		msa::thread::mutex_lock(&hdl->log->queue_mutex);
		hdl->log->messages.push(msg);
		msa::thread::mutex_unlock(&hdl->log->queue_mutex);
	}

	static void *writer_start(void *args)
	{
		msa::Handle hdl = (msa::Handle) args;
		// run util shutdown, and then keep running until the message queue is empty
		while (hdl->log->running)
		{
			Message *msg = writer_poll_msg(hdl);
			if (msg != NULL)
			{
				writer_write_to_streams(hdl, msg);
				dispose_message(msg);
			}
			else
			{
				msa::util::sleep_milli(5);
			}
		}
		msa::thread::mutex_lock(&hdl->log->queue_mutex);
		// empty everything remaining
		while (!hdl->log->messages.empty())
		{
			Message *rem_msg = hdl->log->messages.front();
			hdl->log->messages.pop();
			writer_write_to_streams(hdl, rem_msg);
			dispose_message(rem_msg);
		}
		msa::thread::mutex_unlock(&hdl->log->queue_mutex);
		return NULL;
	}

	static Message *writer_poll_msg(msa::Handle hdl)
	{
		Message *msg = NULL;
		msa::thread::mutex_lock(&hdl->log->queue_mutex);
		if (!hdl->log->messages.empty())
		{
			msg = hdl->log->messages.front();
			hdl->log->messages.pop();
		}
		msa::thread::mutex_unlock(&hdl->log->queue_mutex);
		return msg;
	}

	static void writer_write_to_streams(msa::Handle hdl, const Message *msg)
	{
		for (size_t i = 0; i < hdl->log->streams.size(); i++)
		{
			LogStream *stream = hdl->log->streams.at(i);
			if (msg->level >= stream->level)
			{
				writer_write(stream, msg);
			}
		}
	}

	static void writer_write(LogStream *stream, const Message *msg)
	{
		if (stream->format == Format::XML)
		{
			writer_write_xml(stream, msg);
		}
		else if (stream->format == Format::TEXT)
		{
			writer_write_text(stream, msg);
		}
		else
		{
			throw std::logic_error("bad log stream format");
		}
	}

	static void writer_write_xml(LogStream *stream, const Message *msg)
	{
		char buffer[512];
		struct tm *time_info = new struct tm;
		gmtime_r(&msg->time, time_info);
		const char *time_str = get_time_str(time_info);
		const char *lev_str = level_to_str(msg->level);
		const char *thread_str = msg->thread->c_str();
		const char *text_str = msg->text->c_str();
		const char *format = stream->output_format_string.c_str();
		sprintf(buffer, format, time_str, thread_str, lev_str, text_str);
		delete[] time_str;
		delete time_info;
		*(stream->out) << buffer << std::endl;
	}

	static void writer_write_text(LogStream *stream, const Message *msg)
	{
		char buffer[512];
		struct tm *time_info = new struct tm;
		localtime_r(&msg->time, time_info);
		const char *time_str = get_time_str(time_info);
		const char *lev_str = level_to_str(msg->level);
		const char *thread_str = msg->thread->c_str();
		const char *text_str = msg->text->c_str();
		const char *format = stream->output_format_string.c_str();
		sprintf(buffer, format, time_str, thread_str, lev_str, text_str);
		delete[] time_str;
		delete time_info;
		*(stream->out) << buffer << std::endl;
	}

	static const char *get_time_str(const struct tm *time_info)
	{
		char *buf = new char[26];
		asctime_r(time_info, buf);
		char *ptr = strchr(buf, '\n');
		if (*ptr == '\n')
		{
			*ptr = '\0';
		}
		return buf;
	}

} }
