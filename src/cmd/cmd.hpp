#ifndef MSA_CMD_CMD_HPP
#define MSA_CMD_CMD_HPP

// Handles definitions and specifications of commands

#include "msa.hpp"
#include "cfg/cfg.hpp"
#include "event/handler.hpp"

#include <vector>
#include <string>
#include <map>

namespace msa { namespace cmd {

	class ParamList
	{
		public:
			ParamList();
			ParamList(const std::vector<std::string> &tokens, const std::string &opts);
			const std::string &command() const;
			const std::string &operator[](size_t index) const;
			const std::string &get_arg(size_t index) const;
			size_t arg_count() const;
			bool has_option(char opt) const;
			const std::string &get_option(char opt) const;
			size_t option_count(char opt) const;
			const std::vector<std::string> &all_option_args(char opt) const;
			std::string str() const;
		
		private:
			std::string _command;
			std::vector<std::string> _args;
			std::map<char, std::vector<std::string>> _options;
	};

	class Result
	{
		public:
			Result(int status);
			Result(const std::string &retval);
			Result(int status, const std::string &retval);
			const std::string &value() const;
			int status() const;

		private:
			int _status;
			std::string _value;
		
	};
	
	typedef Result (*CommandHandler)(msa::Handle hdl, const ParamList &args, msa::event::HandlerSync *const sync);
	
	class Command
	{
		public:
			Command(const std::string &invoke, const std::string &desc, const std::string &usage, CommandHandler handler) :
				invoke(invoke),
				desc(desc),
				usage(usage),
				options(""),
				handler(handler)
			{}
			
			Command(const std::string &invoke, const std::string &desc, const std::string &usage, const std::string &options, CommandHandler handler) :
				invoke(invoke),
				desc(desc),
				usage(usage),
				options(options),
				handler(handler)
			{}
		
			std::string invoke;
			std::string desc;
			std::string usage;
			std::string options;
			CommandHandler handler;
	};
	
	extern int init(msa::Handle hdl, const msa::cfg::Section &config);
	extern int quit(msa::Handle hdl);
	extern void register_command(msa::Handle hdl, const Command *cmd);
	extern void unregister_command(msa::Handle hdl, const Command *cmd);
	extern const PluginHooks *get_plugin_hooks();
	
	#define MSA_MODULE_HOOK(retspec, name, ...)	extern retspec name(__VA_ARGS__);
	#include "cmd/hooks.hpp"
	#undef MSA_MODULE_HOOK
	
	struct plugin_hooks_type
	{
		#define MSA_MODULE_HOOK(retspec, name, ...)		retspec (*name)(__VA_ARGS__);
		#include "cmd/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};
} }

#endif
