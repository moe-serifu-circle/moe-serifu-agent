#include "lib.hpp"

namespace msa { namespace lib {

	struct library_type
	{
		Handle handle;
		std::string name;
		std::string path;
	};

	library_error::library_error(const std::string &lib_name, const std::string &what) :
		std::runtime_error(what),
		lib_name(lib_name)
	{}

	const std::string &library_error::name() const
	{
		return lib_name;
	}

} }

#if defined(__WIN32)
	#include "win32.cpp"
#elif defined(__ANDROID__)
	#include "android.cpp"
#else
	#include "unix.cpp"
#endif
