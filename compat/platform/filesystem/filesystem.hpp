#ifndef COMPAT_PLATFORM_FILESYSTEM_FILESYSTEM_HPP
#define COMPAT_PLATFORM_FILESYSTEM_FILESYSTEM_HPP

#include <string>
#include <vector>

namespace msa { namespace file {

	extern void list(const std::string &dir_path, std::vector<std::string> &files);

} }

#endif
