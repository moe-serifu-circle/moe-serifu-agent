#ifndef MSA_PLUGIN_HPP
#define MSA_PLUGIN_HPP

#include "msa.hpp"
#include "cmd/cmd.hpp"
#include "configuration.hpp"

#include <string>
#include <cstdint>
#include <vector>

// will look for a function called 'msa_plugin_register()' which must be a RegisterFunc
// with extern "C" linkage

namespace msa { namespace plugin {

	// gcc has this dumb behavior that runs against the standard where it keeps the macros
	// 'major' and 'minor' around for really no good reason. Also we apparently cannot put
	// this undef series in gcc.hpp... wtf gcc.
	#ifdef major
		#undef major
	#endif
	#ifdef minor
		#undef minor
	#endif

	typedef struct info_type Info;

	typedef const Info *(*RegisterFunc)(const msa::PluginHooks *hooks);
	typedef int (*Func)(msa::Handle hdl, void *plugin_env);
	typedef int (*AddCommandsFunc)(msa::Handle hdl, void *plugin_env, std::vector<msa::cmd::Command *> &new_commands);
	typedef int (*InitFunc)(msa::Handle hdl, void **plugin_env);
	
	typedef struct version_type
	{
		uint32_t major;
		uint32_t minor;
		uint32_t debug;
		uint32_t build;
		version_type() : major(0), minor(0), debug(0), build(0) {}
		version_type(uint32_t major) : major(major), minor(0), debug(0), build(0) {}
		version_type(uint32_t major, uint32_t minor) : major(major), minor(minor), debug(0), build(0) {}
		version_type(uint32_t major, uint32_t minor, uint32_t debug) : major(major), minor(minor), debug(debug), build(0) {}
		version_type(uint32_t major, uint32_t minor, uint32_t debug, uint32_t release) : major(major), minor(minor), debug(debug), build(release) {}
		version_type(const std::string &ver_str) : major(0), minor(0), debug(0), build(0)
		{
			size_t parse_pos;
			std::string ver = ver_str;
			// skip initial 'v'
			if (ver[0] == 'v' || ver[0] == 'V')
			{
				ver = ver.substr(1);
			}
			// parse major
			major = (uint32_t) std::stol(ver, &parse_pos);
			if (parse_pos >= (ver.size() - 2) || ver[parse_pos] != '.')
			{
				return;
			}
			// parse minor
			ver = ver.substr(parse_pos + 1);
			minor = (uint32_t) std::stol(ver, &parse_pos);
			if (parse_pos >= (ver.size() - 2) || ver[parse_pos] != '.')
			{
				return;
			}
			// parse debug
			ver = ver.substr(parse_pos + 1);
			debug = (uint32_t) std::stol(ver, &parse_pos);
			if (parse_pos >= (ver.size() - 2) || ver[parse_pos] != 'b')
			{
				return;
			}
			// parse build
			ver = ver.substr(parse_pos + 1);
			build = (uint32_t) std::stol(ver, &parse_pos);
		}
		
		std::string &to_string(std::string &out_str) const
		{
			out_str.clear();
			out_str = "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(debug);
			out_str += (build != 0) ? ("b" + std::to_string(build)) : "";
			return out_str;
		}
	} Version;
	
	typedef struct function_table_type
	{
		InitFunc init_func;
		Func quit_func;
		Func add_input_devices_func;
		Func add_output_devices_func;
		Func add_agent_props_func;
		AddCommandsFunc add_commands_func;
	} FunctionTable;

	struct info_type
	{
		std::string id;
		std::string name;
		std::vector<std::string> authors;
		Version version;
		const FunctionTable *functions;
		info_type(const std::string &id) :
			id(id),
			name(""),
			authors(),
			version(0, 0, 0, 0),
			functions(NULL)
			{}
		info_type(const std::string &id,
					const std::string &name,
					const std::vector<std::string> &authors,
					const Version version,
					const FunctionTable *funcs) :
			id(id),
			name(name),
			authors(authors),
			version(version),
			functions(funcs)
			{}
	};

	extern int init(msa::Handle hdl, const msa::config::Section &config);
	// call setup only after entire msa system is inited
	extern int setup(msa::Handle hdl);
	// call setup only before entire msa system is quit
	extern int teardown(msa::Handle hdl);
	extern int quit(msa::Handle hdl);
	extern const std::string &load(msa::Handle hdl, const std::string &path);
	extern void unload(msa::Handle hdl, const std::string &id);
	extern void get_loaded(msa::Handle hdl, std::vector<std::string> &ids);
	extern void enable(msa::Handle hdl, const std::string &id);
	extern void disable(msa::Handle hdl, const std::string &id);
	extern bool is_enabled(msa::Handle hdl, const std::string &id);
	extern bool is_loaded(msa::Handle hdl, const std::string &id);
	extern const Info *get_info(msa::Handle hdl, const std::string &id);
	
} }

#endif
