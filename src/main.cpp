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
		fprintf(stderr, "\nMSA init failed");
		if (succ == MSA_ERR_CONFIG)
		{
			fprintf(stderr, ": could not load config file");
		}
		fprintf(stderr, "\n");
		return EXIT_FAILURE;
	}
	printf("Master: \"Waiting for EDT to start...\"\n");
	while (hdl->status == msa::Status::CREATED)
	{
		msa::util::sleep_milli(50);
	}
	printf("Master: \"System is ready.\"\n");
	while (hdl->status == msa::Status::RUNNING)
	{
		msa::util::sleep_milli(50);
	}
	printf("Master: \"Waiting for MSA to exit...\"\n");
	while (hdl->status != msa::Status::STOPPED)
	{
		msa::util::sleep_milli(50);
	}
	if (hdl->status == msa::Status::STOPPED)
	{
		printf("Master: \"MSA system has exited.\"\n");
		msa::dispose(hdl);
	}
	else
	{
		printf("Master: \"MSA state is not shutdown, but still terminating.\n");
	}
	return EXIT_SUCCESS;
}

