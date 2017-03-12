#include "windows.h"

namespace msa { namespace file {

	static const std::string DIR_SEPARATOR = "\\";

	extern void list(const std::string &dir_path, std::vector<std::string> &files)
	{
		std::string scan_criteria = dir_path;
		join(scan_criteria, "*.*");
		WIN32_FIND_DATA find_data;
		HANDLE find_handle = FindFirstFile(scan_criteria.c_str(), &find_data);
		if (find_handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				files.push_back(find_data.cFileName);
			} while(FindNextFile(find_handle, &find_data));
			FindClose(find_handle);
		}
	}

	extern const std::string &dir_separator()
	{
		return DIR_SEPARATOR;
	}

} }
