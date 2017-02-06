#include "msa.hpp"
#include "agent.hpp"
#include "input.hpp"
#include "event/dispatch.hpp"
#include "configuration.hpp"

#include <string>

namespace msa {

	extern int init(Handle *msa, const char *config_path)
	{
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

		int ret;
		std::string sec_name;
		
		sec_name = "EVENT";
		msa::config::Section event_conf(sec_name);
		if (conf->find(sec_name) != conf->end())
		{
			event_conf = (*conf)[sec_name];
		}
		ret = msa::event::init(hdl, event_conf);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_EVENT;
		}

		sec_name = "INPUT";
		msa::config::Section input_conf(sec_name);
		if (conf->find(sec_name) != conf->end())
		{
			input_conf = (*conf)[sec_name];
		}
		ret = msa::input::init(hdl, input_conf);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_INPUT;
		}

		sec_name = "AGENT";
		msa::config::Section agent_conf(sec_name);
		if (conf->find(sec_name) != conf->end())
		{
			agent_conf = (*conf)[sec_name];
		}
		ret = msa::agent::init(hdl, agent_conf);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return MSA_ERR_AGENT;
		}

		*msa = hdl;
		delete conf;
		return MSA_SUCCESS;
	}

	extern int quit(Handle msa)
	{
		int status = 0;

		if (msa->event != NULL)
		{
			status = msa::event::quit(msa);
			if (status != 0)
			{
				return MSA_ERR_EVENT;
			}
			msa->event = NULL;
		}

		if (msa->input != NULL)
		{
			status = msa::input::quit(msa);
			if (status != 0)
			{
				return MSA_ERR_INPUT;
			}
			msa->input = NULL;
		}

		if (msa->agent != NULL)
		{
			status = msa::agent::quit(msa);
			if (status != 0)
			{
				return MSA_ERR_AGENT;
			}
			msa->agent = NULL;
		}

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
		delete msa;
		return MSA_SUCCESS;
	}

}
