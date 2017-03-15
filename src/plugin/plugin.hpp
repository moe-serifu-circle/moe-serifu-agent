#ifndef PLUGIN_PLUGIN_HPP
#define PLUGIN_PLUGIN_HPP

#include "msa.hpp"
#include "cmd/cmd.hpp"
#include "configuration.hpp"

#include <string>
#include <cstdint>
#include <vector>

// will look for a function called 'msa_plugin_getinfo()' which must be a GetInfoFunc with
// extern "C" linkage

namespace msa { namespace plugin {

	typedef struct info_type Info;

	typedef const Info *(*GetInfoFunc)(void);
	typedef int (*Func)(msa::Handle hdl, void *plugin_env);
	typedef int (*AddCommandsFunc)(msa::Handle hdl, void *plugin_env, std::vector<msa::cmd::Command *> &new_commands);
	typedef int (*InitFunc)(msa::Handle hdl, void **plugin_env);
	
	typedef struct version_type
	{
		// some compilers (looking at you, GNU.) clobber the symbols major and minor, which interfers with our
		// init lists for the version struct. Undefine them for now if that is the case and turn them back on
		// after the version struct.
		#ifdef major
			#define __major_temp major
			#undef major
		#endif
		#ifdef minor
			#define __minor_temp minor
			#undef minor
		#endif
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
			// parse major
			std::string ver = ver_str;
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
		// we are done using major and minor in the version code, so turn them back on
		#ifdef __major_temp
			#define major __major_temp
			#undef __major_temp
		#endif
		#ifdef __minor_temp
			#define minor __minor_temp
			#undef __minor_temp
		#endif
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
		std::string name;
		std::vector<std::string> authors;
		Version version;
		const FunctionTable *functions;
		info_type() :
			name(""),
			authors(),
			version(0, 0, 0, 0),
			functions(NULL)
			{}
		info_type(const std::string &name,
					const std::vector<std::string> &authors,
					const Version version,
					const FunctionTable *funcs) :
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
	
} }

#endif
