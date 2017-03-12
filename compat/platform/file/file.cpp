#include "file.hpp"

#if defined(__WIN32)
	#include "win32.cpp"
#elif defined(__ANDROID__)
	#include "android.cpp"
#else
	#include "unix.cpp"
#endif

namespace msa { namespace file {

	extern void join(std::string &base, const std::string &next)
	{
		base += dir_separator() + next;
	}

	extern void basename(std::string &path, const std::string &suffix)
	{
		const std::string sep = dir_separator();
		while (path.substr(path.size() - sep.size()) == sep)
		{
			for (size_t i = 0; i < dir_sep.size(); i++)
			{
				path.erase(path.size() - 1);
			}
		}
		size_t sep_pos = path.rfind(dir_sep);
		if (sep_pos != std::string::npos)
		{
			path = path.substr(sep_pos + dir_sep.size());
		}
		if (suffix != "" && path.size() > suffix.size() && path.substr(path.size() - suffix.size()) == suffix)
		{
			path = path.substr(0, path.size() - suffix.size());
		}
	}

} }

