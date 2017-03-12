#include <dlfcn.h>
#include <type_traits>

#include "comapt/file/file.hpp"

namespace msa { namespace lib {

	extern Library *open(const std::string &path)
	{
		std::string name = path;
		msa::file::basename(name, ".so");
		void *handle_ptr = dlopen(path.c_str(), RTLD_NOW);
		if (handle_ptr == NULL)
		{
			throw library_error(name, "could not load library");
		}
		Library *lib = new Library;
		lib->handle = handle_ptr;
		lib->name = name;
		lib->path = path;
		return lib;
	}
	
	extern void *get_untyped_symbol(Library *lib, const std::string &symbol)
	{
		dlerror();
		void *sym_addr = dlsym(lib->handle, symbol.c_str());
		const char *err_msg = dlerror();
		if (err_msg != NULL)
		{
			throw library_error(lib->name, "could not find symbol: " + symbol);
		}
		return sym_addr;
	}
	
	extern void close(Library *lib)
	{
		int status = dlclose(lib->handle);
		if (status != 0)
		{
			throw library_error(lib->name, "could not close library (dlclose() returned " + std::to_string(status) + ")");
		}
		delete lib;
	}

} }
