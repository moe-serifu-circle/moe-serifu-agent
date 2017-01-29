/* Main source code file. */

#include "control.hpp"
#include "event_handler.hpp"

#include <cstdlib>
#include <cstdio>

#include <unistd.h>

static void say_func(const msa::event::Event *const e, msa::event::HandlerSync *cons);

int main(int argc, char *argv[]) {
	msa::control::Handle hdl;
	if (msa::control::init(&hdl) != 0)
	{
		perror("could not init msa handle");
		return EXIT_FAILURE;
	}
	msa::control::subscribe(hdl, msa::event::Topic::COMMAND_ANNOUNCE, say_func);
	printf("Master: \"subscribed to announce\"\n");
	while (true)
	{
		int seconds_sleep = rand() % 10 + 1;
		printf("Master: \"waiting %d seconds...\"\n", seconds_sleep);
		sleep(seconds_sleep);
		printf("Master: \"Masa-chan, Announce Yourself.\"\n");
		const msa::event::Event *e = msa::event::create(msa::event::Topic::COMMAND_ANNOUNCE, NULL);
		msa::control::push_event(hdl, e);
	}
	return EXIT_SUCCESS;
}

static void say_func(const msa::event::Event *const e, msa::event::HandlerSync *cons)
{
	printf("Masa-chan: \"I'd like to announce my presence!\"\n");
}
