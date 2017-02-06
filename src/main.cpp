/* Main source code file. */

#include "msa.hpp"
#include "event/dispatch.hpp"
#include "cxx_normalization.hpp"
#include "agent.hpp"
#include "util.hpp"

#include <cstdlib>
#include <cstdio>
#include <string>

#include <unistd.h>

static void say_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
static void exit_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

int main(int argc, char *argv[]) {
	const char *cfg_path;
	if (argc < 2)
	{
		printf("no config file given, defaulting to 'msa.cfg'\n");
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
		printf("\nMSA init failed");
		if (succ == MSA_ERR_CONFIG)
		{
			printf(": could not load config file");
		}
		printf("\n");
		return EXIT_FAILURE;
	}
	msa::event::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
	msa::event::subscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
	msa::event::subscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
	printf("Master: \"Subscribed to command hooks\"\n");
	printf("Master: \"Waiting for EDT to start...\"\n");
	while (hdl->status == msa::Status::CREATED)
	{
		msa::util::sleep_milli(50);
	}
	printf("Master: \"System is ready.\"\n");
	while (hdl->status == msa::Status::RUNNING)
	{
		// busy wait, user enters commands
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

static void say_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
{
	const msa::agent::Agent *a = msa::agent::get_agent(hdl);
	printf("%s: \"I'd like to announce my presence!\"\n", a->name.c_str());
}

static void exit_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
{
	const msa::agent::Agent *a = msa::agent::get_agent(hdl);
	const char *name = a->name.c_str();
	printf("%s: \"Right away master, I will terminate my EDT for you now!\"\n", name);
	int status = msa::quit(hdl);
	printf("%s: \"Environment Status: %d\"\n", name, hdl->status);
	if (status == 0)
	{
		printf("%s: \"System shutdown.\"\n", name);
	}
	else
	{
		printf("%s: \"Warning! could not quit: %d\"\n", name, status);
	}
}

static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
{
	const msa::agent::Agent *a = msa::agent::get_agent(hdl);
	std::string *str = static_cast<std::string *>(e->args);
	printf("%s: \"I'm sorry, Master. I don't understand the command '%s'\"\n", a->name.c_str(), str->c_str());
	delete str;
}
