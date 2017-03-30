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

// gcc has this dumb behavior that runs against the standard where it keeps the macros
// 'major' and 'minor' around for really no good reason.
#error "fail"
#ifdef major
	#undef major
#else
	#error "Now I'm just confused"
#endif
#ifdef minor
	#undef minor
#else
	#error "I really don't understand"
#endif
