#if ! defined WIN32UTIL_H
#define WIN32UTIL_H

#include <vector>
#include <string>

#include <windows.h>

bool reg_query_value(HKEY key, const char *entry, DWORD *pType, std::vector<unsigned char> *pValue, LONG *pErrorCode);

std::string get_open_command_for_extension(const std::string &extension); // returns null string when error

#endif // WIN32UTIL_H

