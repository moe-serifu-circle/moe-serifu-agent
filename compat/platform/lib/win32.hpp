#ifndef COMPAT_PLATFORM_LIB_LIB_HPP
	#error "Do not include compat libs directly"
#endif

#include <windows.h>

namespace msa { namespace lib {

	typedef HMODULE Handle;

	template<typename T>
	extern void *get_symbol(Library *lib, const std::string &symbol)
	{
		static_assert(std::is_pointer<T>::value, "get_symbol must return a pointer type");
		dlerror();
		void *ptr = dlsym(lib->handle, symbol.c_str());
		const char *err_msg = dlerror();
		if (err_msg != NULL)
		{
			throw library_error(lib->name, "could not find symbol: " + symbol);
		}
		// get around the different-sized obj/func pointer problem:
		union
		{
			T typed_ptr;
			void *untyped_ptr;
		} alias;
		alias.untyped_ptr = ptr;
		T sym_addr = alias.typed_ptr;
		return sym_addr;
	}

} }

