#include "util.hpp"

namespace msa { namespace util {

	extern void sleep_milli(int millisec) {
		msa::platform::sleep(millisec);
	}

	extern bool check_stdin_ready()
	{
		return msa::platform::select_stdin();
	}

} }
