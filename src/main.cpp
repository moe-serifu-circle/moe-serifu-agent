/* Main source code file. */

#include "environment.hpp"
#include "control.hpp"
#include "event_handler.hpp"

#include <cstdlib>
#include <cstdio>

#include <unistd.h>

static void say_func(const msa::event::Event *const e, msa::event::HandlerSync *sync);
static void exit_func(const msa::event::Event *const e, msa::event::HandlerSync *sync);

static msa::core::Handle hdl;

int main(int argc, char *argv[]) {
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
	while (hdl->status == msa::core::Status::CREATED)
	{
		printf("Master \"Waiting for run...\"\n");
	}
	while (hdl->status == msa::core::Status::RUNNING)
	{
		events_count++;
		printf("Master: \"Masa-chan, Announce Yourself.\"\n");
		msa::event::Event *e = msa::event::create(msa::event::Topic::COMMAND_ANNOUNCE, NULL);
		msa::core::push_event(hdl, e);
		if (events_count >= 2)
		{
			printf("Master: \"Masa-chan, I want you to exit.\"\n");
			msa::event::Event *exit_e = msa::event::create(msa::event::Topic::COMMAND_EXIT, NULL);
			msa::core::push_event(hdl, exit_e);
		}
		int seconds_sleep = rand() % 10 + 1;
		printf("Master: \"waiting %d seconds...\"\n", seconds_sleep);
		sleep(seconds_sleep);
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

static void say_func(const msa::event::Event *const e, msa::event::HandlerSync *cons)
{
	printf("Masa-chan: \"I'd like to announce my presence!\"\n");
}

static void exit_func(const msa::event::Event *const e, msa::event::HandlerSync *cons)
{
	printf("Masa-chan: \"Right away master, I will terminate my EDT for you now!\"\n");
	int status = msa::core::quit(hdl);
	printf("Maka-chan: \"Environment Status: %d\"\n", hdl->status);
	if (status == 0)
	{
		printf("Masa-chan: \"System shutdown.\"\n");
	}
	else
	{
		printf("Masa-chan: \"Warning! could not quit: %d\"\n", status);
	}
}
