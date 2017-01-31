/* Main source code file. */

#include "environment.hpp"
#include "control.hpp"
#include "event_handler.hpp"
#include "input.hpp"

#include <cstdlib>
#include <cstdio>

#include <unistd.h>

static void say_func(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *sync);
static void exit_func(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *sync);

int main(int argc, char *argv[]) {
	msa::core::Handle hdl;
	if (msa::core::init(&hdl) != 0)
	{
		perror("could not init msa handle");
		return EXIT_FAILURE;
	}
	msa::core::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
	printf("Master: \"subscribed to announce\"\n");
	msa::core::subscribe(hdl, msa::event::Topic::COMMAND_EXIT, exit_func);
	printf("Master: \"subscribed to exit\"\n");
	int events_count = 0;
	msa::io::init(hdl);
	while (hdl->status == msa::core::Status::CREATED)
	{
		printf("Master \"Waiting for run...\"\n");
	}
	while (hdl->status == msa::core::Status::RUNNING)
	{
		// busy wait, user enters commands
	}
	while (hdl->status != msa::core::Status::STOPPED)
	{
		printf("Master: \"Waiting for Masa-chan to exit...\"\n");
	}
	if (hdl->status == msa::core::Status::STOPPED)
	{
		printf("Master: \"Masa-chan has exited.\"\n");
		msa::core::dispose(hdl);
	}
	return EXIT_SUCCESS;
}

static void say_func(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *cons)
{
	printf("Masa-chan: \"I'd like to announce my presence!\"\n");
}

static void exit_func(msa::core::Handle hdl, const msa::event::Event *const e, msa::event::HandlerSync *cons)
{
	printf("Masa-chan: \"Right away master, I will terminate my EDT for you now!\"\n");
	int status = msa::core::quit(hdl);
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
