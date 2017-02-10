// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

// This file is included when the current compiler is not one of the detectable types.
// It provides defaults for the compiler-specific macros declared in the other files.

// UNUSED(x) surround a parameter with this to mark it as not used
#ifdef UNUSED
	#error "UNUSED is already def'd, need a new name here"
#else
	#define UNUSED(x) x
#endif
