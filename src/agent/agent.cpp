#include "agent.hpp"
#include "log.hpp"
#include "output.hpp"
#include "var.hpp"

#include <string>
#include <cstdint>
#include <exception>

namespace msa { namespace agent {

	static const PluginHooks HOOKS = {
		#define MSA_MODULE_HOOK(retspec, name, ...)		name,
		#include "agent/hooks.hpp"
		#undef MSA_MODULE_HOOK
	};

	struct agent_context_type
	{
		Agent *agent;
		std::string user_title;
		msa::var::Expander *expander;
	};

	Agent::agent_type(const std::string &n) : name(n), state(State::IDLE), attitude(0), mood(Mood::NORMAL)
	{}

	static int create_agent_context(AgentContext **ctx);
	static int dispose_agent_context(AgentContext *ctx);
	static void read_config(msa::Handle hdl, const msa::cfg::Section &config);
	static void add_default_substitutions(msa::Handle hdl);
	static void remove_default_substitutions(msa::Handle hdl);

	extern int init(msa::Handle hdl, const msa::cfg::Section &config)
	{
		int status = create_agent_context(&hdl->agent);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not create agent context");
		}
		try
		{
			read_config(hdl, config);
		}
		catch (const std::exception &e)
		{
			msa::log::error(hdl, "Could not read agent config: " + std::string(e.what()));
		}
		add_default_substitutions(hdl);
		return 0;
	}

	extern int quit(msa::Handle hdl)
	{
		remove_default_substitutions(hdl);
		int status = dispose_agent_context(hdl->agent);
		if (status != 0)
		{
			msa::log::error(hdl, "Could not dispose agent context");
		}
		return 0;
	}

	extern const PluginHooks *get_plugin_hooks()
	{
		return &HOOKS;
	}

	extern const Agent *get_agent(msa::Handle hdl)
	{
		return hdl->agent->agent;
	}

	extern void say(msa::Handle hdl, const std::string &text)
	{
		std::string output_text = "$AGENT_NAME: \"" + text + "\"\n";
		msa::var::expand(hdl->agent->expander, output_text);
		msa::output::write_text(hdl, output_text);
	}

	extern void register_substitution(msa::Handle hdl, const std::string &name)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::register_internal(ctx->expander, name);
	}

	extern void set_substitution(msa::Handle hdl, const std::string &name, const std::string &value)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::set_value(ctx->expander, name, value);
	}

	extern void unregister_substitution(msa::Handle hdl, const std::string &name)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::unregister_internal(ctx->expander, name);
	}

	extern void get_substitutions(msa::Handle hdl, std::vector<std::string> &subs)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::get_defined(ctx->expander, subs);
	}

	static int create_agent_context(AgentContext **ctx_ptr)
	{
		AgentContext *ctx = new AgentContext;
		ctx->agent = NULL;
		msa::var::create_expander(&ctx->expander);
		*ctx_ptr = ctx;
		return 0;
	}

	static int dispose_agent_context(AgentContext *ctx)
	{
		if (ctx->agent != NULL)
		{
			delete ctx->agent;
		}
		msa::var::dispose_expander(ctx->expander);
		delete ctx;
		return 0;
	}

	static void read_config(msa::Handle hdl, const msa::cfg::Section &config)
	{
		std::string name = std::string(config.get_or("NAME", "DEFAULT_NAME"));
		std::string user_title = std::string(config.get_or("USER_TITLE", "Master"));
		hdl->agent->agent = new Agent(name);
		hdl->agent->user_title = user_title;
	}

	static void add_default_substitutions(msa::Handle hdl)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::register_external(ctx->expander, "USER_TITLE", &ctx->user_title);
		msa::var::register_external(ctx->expander, "AGENT_NAME", &ctx->agent->name);
	}

	static void remove_default_substitutions(msa::Handle hdl)
	{
		AgentContext *ctx = hdl->agent;
		msa::var::unregister_external(ctx->expander, "USER_TITLE");
		msa::var::unregister_external(ctx->expander, "AGENT_NAME");
	}

} }
