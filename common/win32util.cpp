#include "win32util.h"

#include <vector>

bool reg_query_value(HKEY key, const char *entry, DWORD *pType, std::vector<unsigned char> *pValue, LONG *pErrorCode)
{
	HKEY hRequiredKey;
	LONG result = ::RegOpenKeyEx(key, entry, 0, KEY_READ, &hRequiredKey);
	if (result != ERROR_SUCCESS) {
		*pType = 0;
		(*pValue).resize(0);
		*pErrorCode = result;
		return false;
	}

	std::vector<unsigned char> buffer;
	buffer.resize(100);
	while (true) {	
		DWORD type;
		DWORD size = buffer.size();
		LONG result = ::RegQueryValueEx(
			hRequiredKey,
			NULL,
			NULL,
			&type,
			&buffer[0],
			&size);
		if (result == ERROR_MORE_DATA) {
			buffer.resize(buffer.size() * 2);
		}
		else {
			::RegCloseKey(hRequiredKey);

			if (result == ERROR_SUCCESS) {
				*pType = type;
				buffer.resize(size);
				(*pValue).swap(buffer);
			}
			else {
				*pType = 0;
				(*pValue).resize(0);
			}
			*pErrorCode = result;
			return result == ERROR_SUCCESS;
		}
	}
}


std::string get_open_command_for_extension(const std::string &extension)
{
	DWORD type;
	LONG errorCode;
	std::vector<unsigned char> value;
	if (! reg_query_value(HKEY_CLASSES_ROOT, extension.c_str(), &type, &value, &errorCode)) {
		return "";
	}
	std::string fileType;
	if (type == REG_SZ || type == REG_MULTI_SZ || type == REG_EXPAND_SZ) {
		fileType = (const char *)(void *)&value[0];
	}
	else {
		return "";
	}

	std::string openCommandKey = fileType + "\\" + "shell" + "\\" + "open" + "\\" + "command";
	if (! reg_query_value(HKEY_CLASSES_ROOT, openCommandKey.c_str(), &type, &value, &errorCode)) {
		return "";
	}
	std::string command;
	if (type == REG_SZ || type == REG_MULTI_SZ || type == REG_EXPAND_SZ) {
		command = (const char *)(void *)&value[0];
	}
	else {
		return "";
	}

	{
		bool insideQuote = false;
		for (size_t p = 0; p < command.size(); ++p) {
			char ch = command[p];
			if (ch == '\"') {
				insideQuote = ! insideQuote;
			}
			else if (ch == ' ') {
				command = command.substr(0, p);
				break; // for p
			}
		}
	}

	return command;
}

