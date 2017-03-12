#include "platform/file/file.hpp"

namespace msa { namespace lib {

	extern Library *open(const std::string &path)
	{
		std::string name = path;
		msa::file::basename(name, ".so");
		HMODULE handle_ptr = LoadLibrary(path);
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
		FARPROC sym_addr = GetProcAddress(lib->handle, symbol.c_str());
		if (sym_addr != NULL)
		{
			throw library_error(lib->name, "could not find symbol: " + symbol);
		}
		return (void *)sym_addr;
	}
	
	extern void close(Library *lib)
	{
		if (!FreeLibrary(lib->handle))
		{
			throw library_error(lib->name, "could not close library");
		}
		delete lib;
	}

} }
