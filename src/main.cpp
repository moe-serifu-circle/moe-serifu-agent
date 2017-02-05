/* Main source code file. */

#include "msa.hpp"
#include "event/dispatch.hpp"
#include "cxx_normalization.hpp"

#include <cstdlib>
#include <cstdio>
#include <string>

#include <unistd.h>

static void say_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
static void exit_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);
static void bad_command_func(msa::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *const sync);

int main(int argc, char *argv[]) {
	if (argc < 2)
	{
		printf("need config file as argument");
		return 1;
	}

	msa::Handle hdl;
	if (msa::init(&hdl, argv[1]) != 0)
	{
		perror("could not init msa handle");
		return EXIT_FAILURE;
	}
	msa::event::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
	msa::event::subscribe(hdl, msa::event::Topic::INVALID_COMMAND, bad_command_func);
	msa::event::subscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
	printf("Master: \"subscribed to command hooks\"\n");
	while (hdl->status == msa::Status::CREATED)
	{
		printf("Master \"Waiting for run...\"\n");
	}
	while (hdl->status == msa::Status::RUNNING)
	{
		// busy wait, user enters commands
	}
	while (hdl->status != msa::Status::STOPPED)
	{
		printf("Master: \"Waiting for Masa-chan to exit...\"\n");
	}
	if (hdl->status == msa::Status::STOPPED)
	{
		printf("Master: \"Masa-chan has exited.\"\n");
		msa::dispose(hdl);
	}
	return EXIT_SUCCESS;
}

static void say_func(msa::Handle UNUSED(hdl), const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
{
	printf("Masa-chan: \"I'd like to announce my presence!\"\n");
}

static void exit_func(msa::Handle hdl, const msa::event::Event *const UNUSED(e), msa::event::HandlerSync *const UNUSED(sync))
{
	printf("Masa-chan: \"Right away master, I will terminate my EDT for you now!\"\n");
	int status = msa::quit(hdl);
	printf("Masa-chan: \"Environment Status: %d\"\n", hdl->status);
	if (status == 0)
	{
		printf("Masa-chan: \"System shutdown.\"\n");
	}
	else
	{
		printf("Masa-chan: \"Warning! could not quit: %d\"\n", status);
	}
}

static void bad_command_func(msa::Handle UNUSED(hdl), const msa::event::Event *const e, msa::event::HandlerSync *const UNUSED(sync))
{
	std::string *str = static_cast<std::string *>(e->args);
	printf("Masa-chan: \"I'm sorry, Master. I don't understand the command '%s'\"\n", str->c_str());
	delete str;
}
