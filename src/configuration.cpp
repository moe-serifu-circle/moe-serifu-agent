#include "configuration.hpp"

#include <ofstream>
#include <ifstream>

namespace msa { namespace config {
std::wstring readFile(const char* filename)
{
    std::wifstream wif(filename);
    wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    return wss.str();
}

	extern Config *load(const std::string &path)
	{
		ifstream config_file;
		config_file.open(path);
		Config *config = NULL;
		if (!config_file.is_open())
		{
			return config;
		}
		
		config_file.close();
	}

} }
