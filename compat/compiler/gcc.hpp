// DO NOT INCLUDE THIS FILE DIRECTLY
#ifndef COMPAT_COMPAT_HPP
	#error "do not include compatibility files directly; include compat/compat.hpp instead"
#endif

// UNUSED(x) surround a parameter with this to mark it as not used
#ifdef UNUSED
	#error "UNUSED is already def'd, need a new name here"
#else
	#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#endif
