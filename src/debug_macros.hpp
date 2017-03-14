#ifndef DEBUG_MACROS_HPP
#define DEBUG_MACROS_HPP

#include <cstdio>

// Contains application-scope preprocessor macros for debugging.

// Before adding anything here, consider carefully whether it would make a better candidate for a
// msa::util function. It should only be defined as a macro here if it would involve code erasure
// in some conditions, and those conditions are not compiler-specific and are not platform-specific.

#ifdef DEBUG
	#define DEBUG_TEST_VAR 1
#else
	#define DEBUG_TEST_VAR 0
#endif

// MACROS

// Use this function like printf; it will print the msg
// to stderr
#define DEBUG_PRINTF(...)					\
	do {									\
		if (DEBUG_TEST_VAR)					\
		{									\
			fprintf(stderr, __VA_ARGS__);	\
		}									\
	} while (0)

#endif
