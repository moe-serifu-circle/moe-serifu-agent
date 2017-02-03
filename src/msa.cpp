#include "msa.hpp"
#include "agent.hpp"
#include "input.hpp"
#include "event/dispatch.hpp"

#define ERR_NONE 0
#define ERR_EVENT 1
#define ERR_INPUT 2
#define ERR_AGENT 3

namespace msa {

	extern int init(Handle *msa)
	{		
		environment_type *hdl = new environment_type;
		hdl->status = Status::CREATED;
		hdl->event = NULL;
		hdl->input = NULL;
		hdl->agent = NULL;

		int ret;
		
		ret = msa::event::init(hdl);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return ERR_EVENT;
		}

		ret = msa::io::init_input(hdl);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return ERR_INPUT;
		}

		ret = msa::agent::init(hdl);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return ERR_AGENT;
		}

		*msa = hdl;
		return ERR_NONE;
	}

	extern int quit(Handle msa)
	{
		int status = 0;

		if (msa->event != NULL)
		{
			status = msa::event::quit(msa);
			if (status != 0)
			{
				return ERR_EVENT;
			}
			msa->event = NULL;
		}

		if (msa->input != NULL)
		{
			status = msa::io::quit_input(msa);
			if (status != 0)
			{
				return ERR_INPUT;
			}
			msa->input = NULL;
		}

		if (msa->agent != NULL)
		{
			status = msa::agent::quit(msa);
			if (status != 0)
			{
				return ERR_AGENT;
			}
			msa->agent = NULL;
		}

		return ERR_NONE;
	}

	extern int dispose(Handle msa)
	{
		// make sure our modules have been properly quit before deleting the pointer
		if (msa->event != NULL)
		{
			return ERR_EVENT;
		}

		if (msa->input != NULL)
		{
			return ERR_INPUT;
		}

		if (msa>agent != NULL)
		{
			return ERR_AGENT;
		}
		delete msa;
		return 0;
	}

}