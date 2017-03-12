#ifndef COMPAT_PLATFORM_LIB_LIB_HPP
#define COMPAT_PLATFORM_LIB_LIB_HPP

#include <string>
#include <stdexcept>
#include <type_traits>

// functions for loading libraries

namespace msa { namespace lib {

	typedef struct library_type Library;

	class library_error : public std::runtime_error
	{
		public:
			library_error(const std::string &lib_name, const std::string &what);
			const std::string &name() const;
		private:
			const std::string lib_name;
	};

	extern Library *open(const std::string &path);
	extern void *get_untyped_symbol(Library *lib, const std::string &symbol);
	extern void close(Library *lib);

	template<typename T>
	T get_symbol(Library *lib, const std::string &symbol)
	{
		static_assert(std::is_pointer<T>::value, "get_symbol can only be called on pointer type");
		void *untyped = get_untyped_symbol(lib, symbol);
		union
		{
			T typed_ptr;
			void *untyped_ptr;
		} alias;
		alias.untyped_ptr = untyped;
		T sym = alias.typed_ptr;
		return sym;
	}

} }

#if defined(__WIN32)
	#include "win32.hpp"
#elif defined(__ANDROID__)
	#include "android.hpp"
#else
	#include "unix.hpp"
#endif

#endif
