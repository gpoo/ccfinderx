
#include <string>
#include <vector>
#include <algorithm>
#include <boost/optional.hpp>
#include <boost/bind.hpp>

#include <tchar.h>
#include <windows.h>
#include <shlobj.h>

BOOL	SafeSHGetSpecialFolderPath(HWND hwndOwner,LPTSTR lpszPath,int nFolder,BOOL fCreate);
boost::optional<std::string> reg_query_value(const std::string &path, const std::string &key);

#if 0

boost::optional<std::string> get_application_data_path()
{
#if defined _MSC_VER
	std::vector<char> buf;
	buf.resize(MAX_PATH * 8);

	if (SafeSHGetSpecialFolderPath(NULL, &buf[0], CSIDL_APPDATA, false)) {
		return std::string(&buf[0]);
	}
	return boost::optional<std::string>();
#else
	return "~";
#endif
}

#else

boost::optional<std::string> get_application_data_path()
{
#if defined _MSC_VER
	return reg_query_value("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "AppData");
#else
	return "~";
#endif
}

#endif

int main(int argc, char *argv[])
{
	using namespace std;

	vector<string> argvec;
	for_each(argv, argv + argc, boost::bind(&vector<string>::push_back, &argvec, _1));

	string outputFile;
	for (size_t i = 0; i < argvec.size(); ++i) {
		string argi = argvec[i];
		if (argi == "-h") {
			printf("usage: getfolder [-o file]\n");
			return 0;
		}
		else if (argi == "-o") {
			if (! (i + 1 < argvec.size())) {
				fprintf(stderr, "error: option -o requires an argument.\n");
				return 1;
			}
			outputFile = argvec[i + 1];
			i += 1;
		}
		else if (argi.substr(0, 1) == "-") {
			fprintf(stderr, "error: unknown option: '%s'\n", argi.c_str());
			return 1;
		}
	}

	boost::optional<string> s = get_application_data_path();

	if (! s) {
		fprintf(stderr, "error: can't get application data folder\n");
		return 2;
	}

	if (outputFile.empty()) {
		printf("%s\n", (*s).c_str());
	}
	else {
		FILE *pf = fopen(outputFile.c_str(), "w");
		if (! pf) {
			fprintf(stderr, "error: can't create a file: '%s'\n", outputFile.c_str());
			return 2;
		}
		fprintf(pf, "%s\n", (*s).c_str());
		fclose(pf);
	}

	return 0;
}

//http://www.usefullcode.net/2006/12/os.html

BOOL	SafeSHGetSpecialFolderPath(HWND hwndOwner,LPTSTR lpszPath,int nFolder,BOOL fCreate)
{
	BOOL	(CALLBACK* g_pfnSHGetSpecialFolderPath)(HWND,LPTSTR,int,BOOL);
	HRESULT	(CALLBACK* g_pfnSHGetFolderPath)(HWND,int,HANDLE,DWORD,LPTSTR);

	int			i;
	BOOL		ret;
	HRESULT		hr;
	HMODULE		hDLL;
	TCHAR		pszDllFile[][15] = {_T("shfolder.dll"),_T("shell32.dll")};

	if(lpszPath == NULL)
		return	FALSE;
	*lpszPath = NULL;

	ret = FALSE;
	for(i = 0; i < 2; i++)
	{
		hDLL = ::LoadLibrary(pszDllFile[i]);
		if(hDLL == NULL)
			continue;

		#ifdef UNICODE
			(*(FARPROC*)&g_pfnSHGetSpecialFolderPath = ::GetProcAddress(hDLL,"SHGetSpecialFolderPathW"));
			(*(FARPROC*)&g_pfnSHGetFolderPath = ::GetProcAddress(hDLL,"SHGetFolderPathW"));
		#else
			(*(FARPROC*)&g_pfnSHGetSpecialFolderPath = ::GetProcAddress(hDLL,"SHGetSpecialFolderPathA"));
			(*(FARPROC*)&g_pfnSHGetFolderPath = ::GetProcAddress(hDLL,"SHGetFolderPathA"));
		#endif

		if(g_pfnSHGetSpecialFolderPath)
			ret = g_pfnSHGetSpecialFolderPath(hwndOwner,lpszPath,nFolder,fCreate);
		if(ret == FALSE && g_pfnSHGetFolderPath)
		{
			hr = g_pfnSHGetFolderPath(hwndOwner,nFolder | (fCreate ? CSIDL_FLAG_CREATE : 0),NULL,SHGFP_TYPE_DEFAULT,lpszPath);
			ret = (SUCCEEDED(hr)) ? TRUE : FALSE;
		}

		::FreeLibrary(hDLL);

		if(ret)
			return	TRUE;
	}

	return	FALSE;
}

boost::optional<std::string> reg_query_value(const std::string &path, const std::string &key)
{
	HKEY hKey;

	LONG r = ::RegOpenKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, KEY_QUERY_VALUE, &hKey);
	if (r != ERROR_SUCCESS) {
		return boost::optional<std::string>(); // fail
	}

	DWORD dwType;
	DWORD dwSize;
	r = ::RegQueryValueEx(hKey, key.c_str(), NULL, &dwType, NULL, &dwSize);
	if (r != ERROR_SUCCESS) {
		return boost::optional<std::string>(); // fail
	}
	if (dwType != REG_SZ) {
		return boost::optional<std::string>(); // fail
	}

	std::vector<char> buf(dwSize);
	::RegQueryValueEx(hKey, key.c_str(), NULL, &dwType, (LPBYTE)&buf[0], &dwSize);
	if (r != ERROR_SUCCESS) {
		return boost::optional<std::string>(); // fail
	}

	assert(buf.size() >= 1);
	assert(buf[buf.size() - 1] == '\0');
	std::string value(&buf[0], buf.size() - 1);

	::RegCloseKey(hKey);

	return value;
}




