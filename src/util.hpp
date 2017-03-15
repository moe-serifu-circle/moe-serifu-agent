#ifndef MSA_UTIL_HPP
#define MSA_UTIL_HPP

/**
* util.hpp
*
* Contains platform-independent utility functions that can see cross-module
* usage but don't have enough coupled functionality to warrant their own module.
*
* This file (and it's .cpp file) shall not include any other MSA module; if there
* is a temptation to add such a dependency, it may be better to move the function
* that is causing the dependency into the module that it depends on.
*/

namespace msa { namespace util {
	
	extern void sleep_milli(int millisec);
	extern bool check_stdin_ready();

} }

#endif