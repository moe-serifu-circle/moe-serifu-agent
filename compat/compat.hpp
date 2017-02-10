#ifndef COMPAT_COMPAT_HPP
#define COMPAT_COMPAT_HPP

// Provides a unified interface for platform and compiler specific-functions.


// COMPILER

#if defined(_MSC_VER)
	#include "compiler/msvc.hpp"
#elif defined(__GNUC__)
	#include "compiler/gcc.hpp"
#else
	#include "compiler/unknown.hpp"
#endif


// PLATFORM

#if defined(_WIN32) || defined(_WIN32_) || defined(WIN32)
	#include "platform/win32.hpp"
#elif defined(__ANDROID__)
	#include "platform/android.hpp"
#else
	// assume it's unix, or at least something posix-ish
	#include "platform/unix.hpp"
#endif

#endif
