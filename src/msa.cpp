#include "msa.hpp"
#include "input.hpp"
#include "event/dispatch.hpp"

namespace msa {	

	extern int init(Handle *msa)
	{		
		environment_type *hdl = new environment_type;
		hdl->status = Status::CREATED;
		hdl->event = NULL;
		hdl->input = NULL;
		int ret = msa::event::init(hdl);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return ret;
		}
		ret = msa::io::init_input(hdl);
		if (ret != 0)
		{
			quit(hdl);
			dispose(hdl);
			return ret;
		}
		*msa = hdl;
		return 0;
	}

	extern int quit(Handle msa)
	{
		int status = 0;
		if (msa->event != NULL)
		{
			status = msa::event::quit(msa);
			if (status != 0)
			{
				return 1;
			}
			msa->event = NULL;
		}
		if (msa->input != NULL)
		{
			status = msa::io::quit_input(msa);
			if (status != 0)
			{
				return 2;
			}
			msa->input = NULL;
		}
		return 0;
	}

	extern int dispose(Handle msa)
	{
		// make sure our modules have been properly quit before deleting the pointer
		if (msa->event != NULL)
		{
			return 1;
		}
		if (msa->input != NULL)
		{
			return 2;
		}
		delete msa;
		return 0;
	}

}
