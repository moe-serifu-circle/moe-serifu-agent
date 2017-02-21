#include "msa.hpp"
#include "agent.hpp"
#include "input.hpp"
#include "cmd.hpp"
#include "event/dispatch.hpp"
#include "configuration.hpp"
#include "log.hpp"

#include <string>

#include "platform/thread/thread.hpp"

namespace msa {

	static const msa::config::Section &get_module_section(msa::config::Config *conf, const char *name);

	static const msa::config::Section blank_section("");

	extern int init(Handle *msa, const char *config_path)
	{
		msa::thread::init();
		// load config first
		msa::config::Config *conf = msa::config::load(config_path);
		if (conf == NULL)
		{
			return MSA_ERR_CONFIG;
		}

		environment_type *hdl = new environment_type;
		hdl->status = Status::CREATED;
		hdl->event = NULL;
		hdl->input = NULL;
		hdl->agent = NULL;
		hdl->cmd = NULL;
		hdl->log = NULL;

		int ret;

		msa::config::Section log_conf = get_module_section(conf, "LOG");
		ret = msa::log::init(hdl, log_conf);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_LOG;
		}
		
		msa::config::Section event_conf = get_module_section(conf, "EVENT");
		ret = msa::event::init(hdl, event_conf);
		if (ret != 0)
		{
			msa::log::error(hdl, "Failed to start event module");
			msa::log::debug(hdl, "msa::event::init() returned " + std::to_string(ret));
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_EVENT;
		}
		msa::log::trace(hdl, "Started event module");
		
		msa::config::Section input_conf = get_module_section(conf, "INPUT");
		ret = msa::input::init(hdl, input_conf);
		if (ret != 0)
		{
			msa::log::error(hdl, "Failed to start input module");
			msa::log::debug(hdl, "msa::input::init() returned " + std::to_string(ret));
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_INPUT;
		}
		msa::log::trace(hdl, "Started input module");

		msa::config::Section agent_conf = get_module_section(conf, "AGENT");
		ret = msa::agent::init(hdl, agent_conf);
		if (ret != 0)
		{
			msa::log::error(hdl, "Failed to start agent module");
			msa::log::debug(hdl, "msa::agent::init() returned " + std::to_string(ret));
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_AGENT;
		}
		msa::log::trace(hdl, "Started agent module");

		msa::config::Section cmd_conf = get_module_section(conf, "CMD");
		ret = msa::cmd::init(hdl, cmd_conf);
		if (ret != 0)
		{
			msa::log::error(hdl, "Failed to start command module");
			msa::log::debug(hdl, "msa::cmd::init() returned " + std::to_string(ret));
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_CMD;
		}
		msa::log::trace(hdl, "Started command module");

		*msa = hdl;
		delete conf;
		msa::log::info(hdl, "Finished initializing Moe Serifu Agent");
		return MSA_SUCCESS;
	}

	extern int quit(Handle msa)
	{
		msa::log::info(msa, "Moe Serifu Agent is now shutting down...");
		int status = 0;

		if (msa->input != NULL)
		{
			status = msa::input::quit(msa);
			if (status != 0)
			{
				msa::log::error(msa, "Failed to stop input module");
				msa::log::debug(msa, "msa::input::quit() returned " + std::to_string(status));
				return MSA_ERR_INPUT;
			}
			msa->input = NULL;
		}
		msa::log::trace(msa, "Stopped input module");

		if (msa->agent != NULL)
		{
			status = msa::agent::quit(msa);
			if (status != 0)
			{
				msa::log::error(msa, "Failed to stop agent module");
				msa::log::debug(msa, "msa::agent::quit() returned " + std::to_string(status));
				return MSA_ERR_AGENT;
			}
			msa->agent = NULL;
		}
		msa::log::trace(msa, "Stopped agent module");

		if (msa->cmd != NULL)
		{
			status = msa::cmd::quit(msa);
			if (status != 0)
			{
				msa::log::error(msa, "Failed to stop command module");
				msa::log::debug(msa, "msa::cmd::quit() returned " + std::to_string(status));
				return MSA_ERR_CMD;
			}
			msa->cmd = NULL;
		}
		msa::log::trace(msa, "Stopped command module");

		if (msa->event != NULL)
		{
			status = msa::event::quit(msa);
			if (status != 0)
			{
				msa::log::error(msa, "Failed to stop event module");
				msa::log::debug(msa, "msa::event::quit() returned " + std::to_string(status));
				return MSA_ERR_EVENT;
			}
			msa->event = NULL;
		}
		msa::log::trace(msa, "Stopped event module");

		msa::log::info(msa, "Moe Serifu Agent primary modules shutdown cleanly");
		if (msa->log != NULL)
		{
			status = msa::log::quit(msa);
			if (status != 0)
			{
				return MSA_ERR_LOG;
			}
			msa->log = NULL;
		}

		msa::thread::quit();
		return MSA_SUCCESS;
	}

	extern int dispose(Handle msa)
	{
		// make sure our modules have been properly quit before deleting the pointer
		if (msa->event != NULL)
		{
			return MSA_ERR_EVENT;
		}

		if (msa->input != NULL)
		{
			return MSA_ERR_INPUT;
		}

		if (msa->agent != NULL)
		{
			return MSA_ERR_AGENT;
		}

		if (msa->cmd != NULL)
		{
			return MSA_ERR_CMD;
		}

		if (msa->log != NULL)
		{
			return MSA_ERR_LOG;
		}
		
		delete msa;
		return MSA_SUCCESS;
	}

	static const msa::config::Section &get_module_section(msa::config::Config *conf, const char *name)
	{
		const std::string name_str = name;
		if (conf->find(name_str) != conf->end())
		{
			return (*conf)[name_str];
		}
		else
		{
			return blank_section;
		}
	}

}
