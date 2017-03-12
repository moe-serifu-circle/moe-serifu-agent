#include "msa.hpp"

#include "plugin/plugin.hpp"
#include "cmd/types.hpp"

extern const msa::plugin::Info *msa_plugin_getinfo();

namespace dekarrin {

	typedef struct env
	{
		Command *commands;
		size_t num_commands;
	} Env;

	static int init(msa::Handle hdl, void **env);
	static int quit(msa::Handle hdl, void *env);
	static int add_commands(msa::Handle hdl, void *plugin_env, std::vector<Command *> &new_commands);
	static void love_func(msa::Handle hdl, const msa::cmd::ArgList &args, msa::event::HandlerSync *const sync);

	static const msa::plugin::Info plugin_info = {"example-plugin", {"dekarrin"}, 1, {1, 0, 0, 0}, init, quit, NULL, NULL, NULL, add_commands};

	static int init(msa::Handle hdl, void **env)
	{
		Env *my_env = new Env;
		my_env->commands = new Command[1];
		my_env->commands[0] = Command("LOVE", "execute a test function", "", love_func);
		*env = my_env;
		return 0;
	}

	static int quit(msa::Handle hdl, void *env)
	{
		Env *my_env = (Env *) env;
		delete[] my_env->commands;
		delete my_env;
	}

	static int add_commands(msa::Handle hdl, void *env, std::vectro<Command *> &new_commands)
	{
		Env *my_env = (Env *) env;
		for (size_t i = 0; i < 1; i++)
		{
			new_commands.push_back(&my_env->commands[i]);
		}
		return 0;
	}
	
	static void love_func(msa::Handle UNUSED(hdl), const msa::cmd::ArgList & UNUSED(args), msa::event::HandlerSync *const UNUSED(sync))
	{
		printf("worked\n");
	}

}

extern const msa::plugin::Info *msa_plugin_getinfo()
{
	return &dekarrin::plugin_info;
}
