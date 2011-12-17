#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <sys/types.h> 
#include <sys/stat.h> 

#include <boost/format.hpp>
#include <boost/optional.hpp>

#if defined _MSC_VER

#include <windows.h>
#define WIN32_LEAN_AND_MEAN 
#include <io.h> // _findfirst
#include <direct.h> // _chdir
#include <shlobj.h> // SHGetSpecialFolderPathA
#include <process.h>

#include <ctype.h>
#include <mbctype.h>

#include <tchar.h>

#elif defined __GNUC__

#include <sys/dir.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>

#define strcmpi strcmp

#endif

#include "unportable.h"

#if ! defined _MSC_VER

#define _getcwd getcwd
#define _chdir chdir

#endif

#if OS_WIN32

#if _MSC_VER < 1400

bool splitpath(const std:: string &path, std:: string *pDir, std:: string *pFname, std:: string *pExt)
{
	char driveStr[_MAX_DRIVE];
	char dirStr[_MAX_DIR];
	char fnameStr[_MAX_FNAME];
	char extStr[_MAX_EXT];

	_splitpath(path.c_str(), driveStr, dirStr, fnameStr, extStr);
	
	std:: string dir = (const char *)driveStr;
	dir += (const char *)dirStr;
	std:: string fname = (const char *)fnameStr;
	std:: string ext = (const char *)extStr;

	if (path.length() != dir.length() + fname.length() + ext.length()) {
		return false;
	}

	if (pDir != NULL) {
		*pDir = dir;
	}
	if (pFname != NULL) {
		*pFname = fname;
	}
	if (pExt != NULL) {
		*pExt = ext;
	}

	return true;
}

#else

bool splitpath(const std:: string &path, std:: string *pDir, std:: string *pFname, std:: string *pExt)
{
	std::vector<char> buf;
	char *driveStr;
	char *dirStr;
	char *fnameStr;
	char *extStr;
	size_t sectionSize = _MAX_PATH;
	while (true) {
		buf.clear();
		buf.resize(sectionSize * 4, 0);
		driveStr = &buf[sectionSize * 0];
		dirStr = &buf[sectionSize * 1];
		fnameStr = &buf[sectionSize * 2];
		extStr = &buf[sectionSize * 3];
		errno_t err = _splitpath_s(path.c_str(), 
			driveStr, sectionSize - 1,
			dirStr, sectionSize - 1,
			fnameStr, sectionSize - 1,
			extStr, sectionSize - 1);
		if (err == 0) {
			break; // while
		}
		else {
			if (err == ERANGE) {
				sectionSize *= 2;
			}
			else {
				assert(false);
			}
		}
	}

	std:: string dir = (const char *)driveStr;
	dir += (const char *)dirStr;
	std:: string fname = (const char *)fnameStr;
	std:: string ext = (const char *)extStr;

	if (path.length() != dir.length() + fname.length() + ext.length()) {
		return false;
	}

	if (pDir != NULL) {
		*pDir = dir;
	}
	if (pFname != NULL) {
		*pFname = fname;
	}
	if (pExt != NULL) {
		*pExt = ext;
	}

	return true;
}

#endif // #if _MSC_VER < 1400

#else

// may cause problems in a EUC like character encodings.

bool splitpath(const std:: string &path, std:: string *pDir, std:: string *pFname, std:: string *pExt)
{
	size_t p = path.rfind('/');
	if (pDir != NULL) {
		*pDir = p != std:: string::npos ? path.substr(0, p + 1) : "";
	}
	std:: string fileName = p != std:: string::npos ? path.substr(p + 1) : path;
	
	size_t q = fileName.rfind('.');
	if (q != std:: string::npos) {
		if (pFname != NULL) {
			*pFname = fileName.substr(0, q);
		}
		if (pExt != NULL) {
			*pExt = fileName.substr(q);
		}
	}	
	else {
		if (pFname != NULL) {
			*pFname = fileName;
		}
		if (pExt != NULL) {
			*pExt = "";
		}
	}

	return true;
}

#endif

#if defined _MSC_VER

std:: string make_temp_file_on_the_same_directory(const std:: string &filePath, const std:: string &base, const std:: string &ext)
{
	std:: string dirStr;

	if (! filePath.empty()) {
		std:: string fnameStr;
		std:: string extStr;
		splitpath(filePath.c_str(), &dirStr, &fnameStr, &extStr);
	}
	else {
		dirStr = "";
	}

	DWORD processID = ::GetCurrentProcessId();

	unsigned long t;
	{
		LARGE_INTEGER largeInt;
		if (::QueryPerformanceCounter(&largeInt)) {
			t = (unsigned long)largeInt.QuadPart;
		}
		else {
			__time64_t t64;
			_time64(&t64);
			t = (unsigned long)t64;
		}
	}

	std:: string newFileName = (boost::format("%s%s-%x%x%s") % dirStr % base % (unsigned long) processID % t % ext).str();
	return newFileName;
}

std:: string make_filename_on_the_same_directory(const std:: string &fileName, const std:: string &targetPathFile)
{
	std:: string dirStr;
	std:: string fnameStr;
	std:: string extStr;

	splitpath(targetPathFile.c_str(), &dirStr, &fnameStr, &extStr);

	std:: string newFileName = (boost::format("%s%s") % dirStr % fileName).str();
	return newFileName;
}

#elif defined __GNUC__

std:: string make_temp_file_on_the_same_directory(const std:: string &filePath, const std:: string &base, const std:: string &ext)
{
	std:: string dirStr;
	std:: string fnameStr;
	std:: string extStr;

	splitpath(filePath.c_str(), &dirStr, &fnameStr, &extStr);
	pid_t pid = getpid();

	time_t timer;
	time(&timer);

	std:: string newFileName = (boost::format("%s%s-%x%x%s") % dirStr % base % (unsigned long) pid % (unsigned long) timer % ext).str();
	return newFileName;
}

std:: string make_filename_on_the_same_directory(const std:: string &fileName,  const std:: string &targetPathFile)
{
	std:: string dirStr;
	std:: string fnameStr;
	std:: string extStr;

	splitpath(targetPathFile.c_str(), &dirStr, &fnameStr, &extStr);

	std:: string newFileName = (boost::format("%s%s") % dirStr % fileName).str();
	return newFileName;
}

#endif

bool PathTime::getFileMTime(const std:: string &path, PathTime *pPathTime)
{
	struct stat t;
	int r = stat(path.c_str(), &t);
	if (r != 0) {
		return false; // fail
	}

	PathTime pt;
	pt.path = path;
	pt.mtime = t.st_mtime;

	*pPathTime = pt;

	return true;
}

std::string file_separator()
{
#if defined OS_WIN32
	return "\\";
#elif defined OS_UNIX
	return "/";
#endif
}

bool path_is_relative(const std:: string &path)
{
	if (path.length() >= 1 && path[0] == '/') {
		return false;
	}
#if defined OS_WIN32
	if (path.length() >= 1 && path[0] == '\\') {
		return false;
	}
	std::string::size_type pos = path.find(":");
	if (pos != std::string::npos && pos + 1 < path.length() && (path[pos + 1] == '\\' || path[pos + 1] == '/')) {
		return false;
	}
#endif
	return false;
}

//void split_path(const std::string &path, std::string *pDirectory, std::string *pFileName)
//{
//#if defined OS_WIN32
//	size_t pos = path.rfind('\\');
//#else
//	size_t pos = path.rfind('/');
//#endif
//	if (pos != std::string::npos) {
//		if (pDirectory != NULL) {
//#if defined OS_WIN32
//			if (pos >= 2 && path[pos - 1] == ':') {
//				*pDirectory = path.substr(0, pos + 1);
//			}
//			else {
//				*pDirectory = path.substr(0, pos);
//			}
//#else
//			*pDirectory = path.substr(0, pos);
//#endif
//			if ((*pDirectory).length() == 0) {
//				*pDirectory = path.substr(0, pos + 1);
//			}
//		}
//		if (pFileName != NULL) {
//			*pFileName = path.substr(pos + 1);
//		}
//	}
//	else {
//		if (pDirectory != NULL) {
//			*pDirectory = "";
//		}
//		if (pFileName != NULL) {
//			*pFileName = path;
//		}
//	}
//}

#if defined OS_WIN32

std::string escape_spaces(const std::string &str)
{
	if (str.find(' ') != std::string::npos) {
		if (str.length() > 0 && str[0] != '\"') {
			return "\"" + str + "\"";
		}
	}
	return str;
}

#elif defined OS_LINUX || defined OS_UBUNTU

std::string escape_spaces(const std::string &str)
{
	std::string r;

	for (size_t i = 0; i < str.length(); ++i) {
		int ch = str[i];
		switch (ch) {
		case ' ':
			r += "\\" " ";
			break;
		default:
			r += ch;
			break;
		}
	}

	return r;
}

#else
#error NO OS IS SPECIFIED
#endif


std::string get_application_data_path()
{
#if defined _MSC_VER
	boost::optional<std::string> reg_query_value(const std::string &path, const std::string &key);

	boost::optional<std::string> v = reg_query_value("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "AppData");
	if (v) {
		return *v + "\\" + "www.ccfinder.net" + "\\" + "CCFinderX";
	}
	return "";
#else
	return "~/.CCFinderX";
#endif
}

namespace {

#if defined _MSC_VER

const char *last_char(const char *str0)
{
	const unsigned char *str = (const unsigned char *)str0;
	const unsigned char *p = str;
	if (*p == '\x0') {
		return NULL;
	}

	const unsigned char *pos = p;
	while (*p != '\x0') {
		if (_ismbslead(pos, p) != 0) {
			pos = p;
			for (++p; *p != '\x0' && _ismbstrail(pos, p) != 0; ++p)
				NULL;
		}
		else {
			pos = p;
			++p;
		}
	}

	return (const char *)pos;
}

#elif defined __GNUC__

// may cause problems in a EUC like character encodings.

const char *last_char(const char *str0)
{
	const unsigned char *str = (const unsigned char *)str0;
	const unsigned char *p = str;
	if (*p == '\x0') {
		return NULL;
	}

	const unsigned char *pos = p;
	while (*p != '\x0') {
		assert('\x0' <= *p && *p <= '\x7e');
		pos = p;
		++p;
	}

	return (const char *)pos;
}

#endif

#if defined _MSC_VER

void find_files_i(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &extensions,
		const std:: string &currentDir)
{
	struct _finddata_t c_file;
	long hFile;

	/* Find first .c file in current directory */
	if((hFile = _findfirst("*.*", &c_file)) != -1) {
		std:: vector<std:: string> files;
		std:: vector<std:: string> directories;
		while (true) {
			std:: string fileName(c_file.name);
			if (fileName == ".") {
				// do nothing
			}
			else if (fileName == "..") {
				// do nothing
			}
			else if ((c_file.attrib & _A_SUBDIR) != 0) {
				directories.push_back(fileName);
			}
			else {
				files.push_back(fileName);
			}
			if (_findnext( hFile, &c_file ) != 0) {
				break; // while true
			}
		}

		std:: sort(directories.begin(), directories.end());
		for (size_t i = 0; i < directories.size(); ++i) {
			const std:: string &fileName = directories[i];
			std:: string dir = currentDir + fileName;
			const char *lastp = last_char(dir.c_str());
			if (lastp != NULL) {
				int ch = *lastp;
				if (dir.length() > 0 && ! (ch == '/' || ch == '\\')) {
					dir += "\\";
				}
				if (_chdir(fileName.c_str()) != -1) {
					find_files_i(pFiles, extensions, dir);
					_chdir("..");
				}
			}
		}
		std:: sort(files.begin(), files.end());
		for (size_t i = 0; i < files.size(); ++i) {
			const std:: string &fileName = files[i];
			std:: string filePath = currentDir + fileName;
			for (std:: set<std:: string>::const_iterator i = extensions.begin(); i != extensions.end(); ++i) {
				std:: string ext = *i;
				if (filePath.length() >= ext.length() && strcmpi(filePath.c_str() + filePath.length() - ext.length(), ext.c_str()) == 0) {
					(*pFiles).push_back(filePath);
					break; // for i
				}
			}
		}
		_findclose( hFile );
	}
}

void find_named_directories_i(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &names,
		const std:: string &currentDir)
{
	struct _finddata_t c_file;
	long hFile;

	/* Find first .c file in current directory */
	if((hFile = _findfirst("*.*", &c_file)) != -1) {
		std:: vector<std:: string> directories;
		while (true) {
			std:: string fileName(c_file.name);
			if (fileName == ".") {
				// do nothing
			}
			else if (fileName == "..") {
				// do nothing
			}
			else if ((c_file.attrib & _A_SUBDIR) != 0) {
				directories.push_back(fileName);
			}
			if (_findnext( hFile, &c_file ) != 0) {
				break; // while true
			}
		}

		std:: sort(directories.begin(), directories.end());
		for (size_t i = 0; i < directories.size(); ++i) {
			const std:: string &fileName = directories[i];
			std:: string dir = currentDir + fileName;
			if (names.find(fileName) != names.end()) {
				(*pFiles).push_back(dir);
			}
			const char *lastp = last_char(dir.c_str());
			if (lastp != NULL) {
				int ch = *lastp;
				if (dir.length() > 0 && ! (ch == '/' || ch == '\\')) {
					dir += "\\";
				}
				if (_chdir(fileName.c_str()) != -1) {
					find_named_directories_i(pFiles, names, dir);
					_chdir("..");
				}
			}
		}
		_findclose( hFile );
	}
}

#elif defined __GNUC__

// 注意！：ディレクトリがソースファイルより前に来るようにはなってない

void find_files_i(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &extensions,
		const std:: string &currentDir)
{
	DIR *dp;
	struct dirent *dir;
	struct stat fi;
	
	if ((dp = opendir(currentDir.c_str())) == NULL) {
		assert(("fail to open directory", false)); // 2007/11/22
	}
	
	while ((dir = readdir(dp)) != NULL) {
		if (dir->d_ino == 0) {
			continue;
		}
		stat(dir->d_name, &fi);
		std:: string fileName = dir->d_name;
		if (S_ISDIR(fi.st_mode)) {
			if (! (fileName == "." || fileName == "..")) {
				std:: string dir = currentDir + fileName;
				if (dir.length() > 0 && dir[dir.length() - 1] != '/') {
					dir += "/";
				}
				if (_chdir(escape_spaces(fileName).c_str()) != -1) {
					find_files_i(pFiles, extensions, dir);
					_chdir("..");
				}
			}
		}
		else {
			std:: string filePath = currentDir + fileName;
			for (std:: set<std:: string>::const_iterator i = extensions.begin(); i != extensions.end(); ++i) {
				std:: string ext = *i;
				std:: string fileExt = filePath.substr(filePath.length() - ext.length());
				if (filePath.length() >= ext.length() && strcmpi(filePath.c_str() + filePath.length() - ext.length(), ext.c_str()) == 0) {
					(*pFiles).push_back(filePath);
					break; // for i
				}
			}
		}
	}
	
	closedir(dp); // 2007/11/22
}

void find_named_directories_i(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &names,
		const std:: string &currentDir)
{
	DIR *dp;
	struct dirent *dir;
	struct stat fi;
	
	if ((dp = opendir(currentDir.c_str())) == NULL) {
		assert(("fail to open directory", false)); // 2007/11/22
	}
	
	while ((dir = readdir(dp)) != NULL) {
		if (dir->d_ino == 0) {
			continue;
		}
		stat(dir->d_name, &fi);
		std:: string fileName = dir->d_name;
		if (S_ISDIR(fi.st_mode)) {
			if (! (fileName == "." || fileName == "..")) {
				std:: string dir = currentDir + fileName;
				if (names.find(fileName) != names.end()) {
					(*pFiles).push_back(dir);
				}
				if (dir.length() > 0 && dir[dir.length() - 1] != '/') {
					dir += "/";
				}
				if (_chdir(escape_spaces(fileName).c_str()) != -1) {
					find_named_directories_i(pFiles, names, dir);
					_chdir("..");
				}
			}
		}
	}
	
	closedir(dp);
}

#endif

}; // namespace

bool find_files(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &extensions, 
		const std:: string &directory)
{
	(*pFiles).clear();

	char *szCurrentDir = _getcwd(NULL, 0);
	if (szCurrentDir == NULL) {
		return false; // failure
	}
	std:: string currentDir(szCurrentDir);
	free(szCurrentDir);

	std:: string dir = directory;
	int ch;
	if (dir.length() > 0 && ! ((ch = dir[dir.length() - 1]) == '/' || ch == '\\')) {
#if defined OS_WIN32
		dir += "\\";
#elif defined OS_UNIX || defined OS_UBUNTU
		dir += "/";
#else
#error
#endif
	}
	if (_chdir(dir.c_str()) == -1) {
		return false; // failure
	}

	find_files_i(pFiles, extensions, dir);

	_chdir(currentDir.c_str());

	return true;
}

bool find_named_directories(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &names, 
		const std:: string &directory)
{
	(*pFiles).clear();

	char *szCurrentDir = _getcwd(NULL, 0);
	if (szCurrentDir == NULL) {
		return false; // failure
	}
	std:: string currentDir(szCurrentDir);
	free(szCurrentDir);

	std:: string dir = directory;
	int ch;
	if (dir.length() > 0 && ! ((ch = dir[dir.length() - 1]) == '/' || ch == '\\')) {
#if defined OS_WIN32
		dir += "\\";
#elif defined OS_UNIX || defined OS_UBUNTU
		dir += "/";
#else
#error
#endif
	}
	if (_chdir(dir.c_str()) == -1) {
		return false; // failure
	}

	find_named_directories_i(pFiles, names, dir);

	_chdir(currentDir.c_str());

	return true;
}

#if defined LITTLE_ENDIAN

void flip_endian(void *pIntegerValue, size_t sizeOfIntegerType)
{
}

#else

void flip_endian(void *pIntegerValue, size_t sizeOfIntegerType)
{
	unsigned char *buffer = (unsigned char *)(void *)pValue;
	unsigned char *p = buffer;
	unsigned char *q = buffer + sizeOfIntegerType - 1;
	for (size_t i = 0; i < sizeOfIntegerType / 2; ++i) {
		unsigned char b = *p; *p = *q; *q = b;
		++p;
		--q;
	}
}

//void flip_endian(int *pValue)
//{
//	unsigned char *buffer = (unsigned char *)(void *)pValue;
//	unsigned char *p = buffer;
//	unsigned char *q = buffer + sizeof(int) - 1;
//	for (size_t i = 0; i < sizeof(int) / 2; ++i) {
//		unsigned char b = *p; *p = *q; *q = b;
//		++p;
//		--q;
//	}
//}
//
//void flip_endian(long *pValue)
//{
//	unsigned char *buffer = (unsigned char *)(void *)pValue;
//	unsigned char *p = buffer;
//	unsigned char *q = buffer + sizeof(long) - 1;
//	for (size_t i = 0; i < sizeof(long) / 2; ++i) {
//		unsigned char b = *p; *p = *q; *q = b;
//		++p;
//		--q;
//	}
//}
//
//void flip_endian(long long *pValue)
//{
//	unsigned char *buffer = (unsigned char *)(void *)pValue;
//	unsigned char *p = buffer;
//	unsigned char *q = buffer + sizeof(long long) - 1;
//	for (size_t i = 0; i < sizeof(long long) / 2; ++i) {
//		unsigned char b = *p; *p = *q; *q = b;
//		++p;
//		--q;
//	}
//}

#endif

#if defined _MSC_VER

void nice(int value)
{
	if (value > 10) {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	}
	else if (value == 10) {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	}
	else {
		::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	}
}

bool allocWorkingSetMemory(long long size)
{
	BOOL success = ::SetProcessWorkingSetSize(::GetCurrentProcess(), 1, size);
	return success != 0;
}

#endif

#if defined OS_WIN32

std::string build_system_string(const std::vector<std::string> &argv)
{
	std::string buffer;
	for (size_t i = 0; i < argv.size(); ++i) {
		if (i > 0) {
			buffer.append(" ");
		}
		std::string argvi = argv[i];
		if (argvi.find(' ') != std::string::npos) {
			if (argvi.length() > 0 && argvi[0] != '\"') {
				argvi = "\"" + argvi + "\"";
			}
		}
		buffer.append(argvi);
	}
	return buffer;
}

#else

std::string build_system_string(const std::vector<std::string> &argv)
{
	std::string buffer;
	for (size_t i = 0; i < argv.size(); ++i) {
		if (i > 0) {
			buffer.append(" ");
		}
		std::string argvi = argv[i];
		buffer.append(argvi);
	}
	return buffer;
}

#endif

// may cause problems in a EUC like character encodings.

std::string join_path(const std::string &s1, const std::string &s2)
{
	if (s1.length() >= 1 && *last_char(s1.c_str()) == '/') {
		return s1 + s2;
	}
#if defined OS_WIN32
	if (s1.length() >= 1 && *last_char(s1.c_str()) == '\\') {
		return s1 + s2;
	}
#endif

	return s1 + file_separator() + s2;
}

bool path_exists(const std::string &path)
{
	struct stat fileStatus;
	if (stat(path.c_str(), &fileStatus) != 0) {
		return false;
	}
	return true;
}

bool path_is_file(const std::string &path)
{
	struct stat fileStatus;
	if (stat(path.c_str(), &fileStatus) != 0) {
		return false; // error (maybe file doesn't exist
	}
	
	return (fileStatus.st_mode & S_IFMT) == S_IFREG;
}

#if defined __GNUC__

int systemv(const std::vector<std::string> &argv)
{
	std::string buf;

	for (size_t i = 0; i < argv.size(); ++i) {
		std::string s = escape_spaces(argv[i]);
		buf += s;
		buf += " ";
	}
	if (buf.length() > 0) {
		buf.erase(buf.length() - 1);
	}

	return system(buf.c_str());
}

#endif

boost::optional<std::string> getenvironmentvariable(const std::string &name)
{
#if defined _MSC_VER && _MSC_VER >= 1400
	char *buf;
	errno_t err = _dupenv_s(&buf, NULL, name.c_str());
	assert(err == 0);
	if (buf != NULL) {
		boost::optional<std::string> s;
		s = buf;
		free(buf);
		return s;
	}
	return boost::optional<std::string>();
#else
	const char *p = getenv(name.c_str());
	if (p != NULL) {
		boost::optional<std::string> s;
		s = p;
		return s;
	}
	return boost::optional<std::string>();
#endif
}


#if defined _MSC_VER

boost::optional<std::string> get_short_path_name(const std::string &path0)
{
	std::string path = path0;

	DWORD bufSize = ::GetShortPathName(path.c_str(), NULL, 0);
	if (bufSize == 0) {
		std::string dir;
		std::string fname;
		std::string ext;
		if (! splitpath(path, &dir, &fname, &ext)) {
			return boost::optional<std::string>(); // fail
		}
		if (dir.empty() || fname.empty() && ext.empty()) {
			return boost::optional<std::string>(); // fail
		}
		boost::optional<std::string> t = get_short_path_name(dir);
		if (t) {
			path = *t + fname + ext;
		}
		bufSize = ::GetShortPathName(path.c_str(), NULL, 0);
		if (bufSize == 0) {
			return boost::optional<std::string>(); // fail
		}
	}
	std::vector<char> buf(bufSize + 1);
	DWORD strLen = ::GetShortPathName(path.c_str(), &buf[0], bufSize);
	if (strLen == 0) {
		return boost::optional<std::string>(); // fail
	}
	assert(strLen <= bufSize);
	boost::optional<std::string> r;
	r = std::string((char *)&buf[0], strLen);
	return r;
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

#endif
