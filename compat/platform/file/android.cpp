#include <dirent.h>
#include <errno.h>

#include <string>

namespace msa { namespace file {

	extern void list(const std::string &dir_path, std::vector<std::string> &files)
	{
		DIR *d = opendir(dir_path.c_str());
		if (d == NULL)
		{
			throw std::logic_error("could not open dir (" + std::to_string(errno) + "): " + dir_path);
		}
		struct dirent *entry;
		while ((entry = readdir(d)) != NULL)
		{
			files.push_back(std::string(entry->d_name));
		}
		closedir(d);
	}

	extern void join(std::string &base, const std::string &next)
	{
		base += "/" + next;
	}

} }
