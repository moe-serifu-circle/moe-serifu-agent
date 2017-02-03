// contains macros for normalizing c++ compiler behavior

#ifndef CXX_NORMALIZATION_HPP
#define CXX_NORMALIZATION_HPP

// UNUSED(x) surround a parameter with this to mark it as not used
#ifdef UNUSED
	#error "UNUSED is already def'd, need a new name here"
#else
	#if defined(_MSC_VER)
		#define UNUSED(x) __pragma(warning(suppress:4100)) x
	#elif defined(__GNUC__)
		#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
	#else
		#define UNUSED(x) x
	#endif
#endif
	


#endif