/* Main source code file. */

#include "msa.hpp"
#include "util.hpp"

#include <cstdlib>
#include <cstdio>

int main(int argc, char *argv[]) {
	const char *cfg_path;
	if (argc < 2)
	{
		fprintf(stderr, "no config file given, defaulting to 'msa.cfg'\n");
		cfg_path = "msa.cfg";
	}
	else
	{
		cfg_path = argv[1];
	}

	msa::Handle hdl;
	int succ = msa::init(&hdl, cfg_path);
	if (succ != MSA_SUCCESS)
	{
		const char *err_msg;
		switch (succ)
		{
			case MSA_ERR_CONFIG:
				err_msg = "could not load config file";
				break;
			case MSA_ERR_LOG:
				err_msg = "could not init logging system";
				break;
			case MSA_ERR_INPUT:
				err_msg = "could not start input system";
				break;
			case MSA_ERR_OUTPUT:
				err_msg = "could not start output system";
				break;
			case MSA_ERR_EVENT:
				err_msg = "could not start event dispatcher";
				break;
			case MSA_ERR_AGENT:
				err_msg = "could not init agent state machine";
				break;
			case MSA_ERR_CMD:
				err_msg = "could not start command hooks";
				break;
			default:
				err_msg = "unknown problem";
				break;
		}
		fprintf(stderr, "\nMSA init failed: error %d - %s\n", succ, err_msg);
		// if we got a config error or a log error, the log has not yet been started
		if (succ != MSA_ERR_CONFIG && succ != MSA_ERR_LOG)
		{
			fprintf(stderr, "See log for more details\n");
		}
		return EXIT_FAILURE;
	}
	DEBUG_PRINTF("(Waiting for EDT to start)\n");
	while (hdl->status == msa::Status::created)
	{
		msa::util::sleep_milli(50);
	}
	DEBUG_PRINTF("(System is ready)\n");
	while (hdl->status == msa::Status::running)
	{
		msa::util::sleep_milli(50);
	}
	DEBUG_PRINTF("(Waiting for MSA to exit)\n");
	while (hdl->status != msa::Status::stopped)
	{
		msa::util::sleep_milli(50);
	}
	if (hdl->status == msa::Status::stopped)
	{
		DEBUG_PRINTF("(MSA system has exited)\n");
		msa::dispose(hdl);
	}
	else
	{
		DEBUG_PRINTF("(MSA state is not shutdown, but still terminating)\n");
	}
	return EXIT_SUCCESS;
}

