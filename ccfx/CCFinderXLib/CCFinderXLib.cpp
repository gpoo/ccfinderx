#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/optional.hpp>

#if 0
#include <locale.h>
#endif

#include <stdlib.h> // getenv()

#if defined _MSC_VER
#include <process.h>
#include <shlobj.h>
#include <windows.h>
#endif

#if defined __GNUC__
#include <dirent.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#endif

#include "../../common/argvbuilder.h"
#include "../../common/unportable.h"
#include "../../common/utf8support.h"
#include "../../GemX/ccfinderx_CCFinderX.h"
#include "../../common/base64encoder.h"
#include "../ccfxcommon.h"
#include "../ccfxconstants.h"

#if defined _MSC_VER
#include "../../common/win32util.h"
#endif

#if 0
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

#if 0
class OemConv {
private:
	_locale_t oemLocale;
public:
	OemConv()
		: oemLocale(::_create_locale(LC_ALL, ".OCP"))
	{
	}
	~OemConv()
	{
		::_free_locale(oemLocale);
	}
public:
	std::basic_string<wchar_t> mbstowcs(const std::string &mbs)
	{
		size_t len = ::mbstowcs(NULL, mbs.c_str(), 0, oemLocale);
		std::vector<wchar_t> buf;
		buf.resize(len + 1);
		size_t len2 = ::mbstowcs(&buf[0], mbs.c_str(), len, oemLocale);
		assert(len2 == len);

		return std::basic_string<wchar_t>(&buf[0], len);
	}

	std::string wcstombs(const std::basic_string<wchar_t> &wcs)
	{
		size_t len = ::wcstombs(NULL, wcs.c_str(), 0, oemLocale);
		std::vector<char> buf;
		buf.resize(len + 1);
		size_t len2 = ::wcstombs(&buf[0], wcs.c_str(), len, oemLocale);
		assert(len2 == len);
		return std::string(&buf[0], len);
	}
};
#endif

//static PreprocessPackLookup preprocessPackLookup; // 2008/04/02

static boost::optional<std::string> oModuleDir;

int exec_ccfx(const std:: vector<std:: string> &argsUtf8);

#if defined _MSC_VER
std::string getPathWithoutSpace(const std::string &dir)
{
	if (dir.find(' ') == std::string::npos) {
		return dir;
	}

	std::string dirSys = INNER2SYS(dir);
	DWORD bufSize = ::GetShortPathName(dirSys.c_str(), NULL, 0);
	if (bufSize == 0) {
		return std::string(); // fail
	}
	std::vector<char> buf(bufSize + 1);
	DWORD strLen = ::GetShortPathName(dirSys.c_str(), &buf[0], bufSize);
	if (strLen == 0) {
		return std::string(); // fail
	}
	assert(strLen <= bufSize);
	std::string str((char *)&buf[0], strLen);

	return SYS2INNER(str);
}
#endif

JNIEXPORT void JNICALL Java_ccfinderx_CCFinderX_setModuleDirectory
  (JNIEnv *env, jobject, jstring strDir)
{
	const char* utf8StrDir = env->GetStringUTFChars(strDir, NULL);
	std:: string str = utf8StrDir;

#if defined _MSC_VER
	str = getPathWithoutSpace(str);
	if (str.empty()) { // error!
		str = utf8StrDir;
	}
#endif	
	oModuleDir = str;

	env->ReleaseStringUTFChars(strDir, utf8StrDir); 
}

JNIEXPORT jstring JNICALL Java_ccfinderx_CCFinderX_getCCFinderXPath
  (JNIEnv *env, jobject)
{
	assert(!! oModuleDir);

#if defined OS_WIN32
	std:: string exeFile = *oModuleDir + file_separator() + "ccfx.exe";
#elif defined OS_UNIX
	std:: string exeFile = *oModuleDir + file_separator() + "ccfx";
#endif

	std:: string exeFileUtf8 = SYS2INNER(exeFile);
	//for (size_t i = 0; i < exeFile.length(); ++i) {
	//	int ch = exeFile[i];
	//	if (! (0x20 <= ch && ch <= 0x7e)) {
	//		exeFile = ""; // invaid path
	//	}
	//}

	jstring result = env->NewStringUTF(exeFileUtf8.c_str());
	return result;
}

JNIEXPORT jint JNICALL Java_ccfinderx_CCFinderX_invokeCCFinderX
  (JNIEnv *env, jobject caller, jobjectArray args)
{
	jsize length = env->GetArrayLength(args);

	std:: vector<std:: string> strs;
	for (size_t i = 0; i < length; ++i) {
		jstring jstr = (jstring)env->GetObjectArrayElement(args, i);
		const char *uniStr = env->GetStringUTFChars(jstr, NULL);
		std:: string str = uniStr;

		if (str.length() > 0 && str[0] != '\"' && str.find(' ') != std:: string::npos) {
			str = "\"" + str + "\"";
		}

		strs.push_back(str);
		env->ReleaseStringUTFChars(jstr, uniStr);
	}

	return exec_ccfx(strs);
}

JNIEXPORT jintArray JNICALL Java_ccfinderx_CCFinderX_getVersion
  (JNIEnv *env, jobject caller)
{
	{
		std:: vector<std:: string> args;
		int r = exec_ccfx(args);
	}

	jintArray ary = env->NewIntArray(3);
	
	jint version[3] = { APPVERSION[0], APPVERSION[1], APPVERSION[2] };
	env->SetIntArrayRegion(ary, 0, 3, version);

	return ary;
}

void openUrl(const std::string &url)
{
#if defined OS_WIN32
	::ShellExecute(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif defined OS_UNIX
	::system(("firefox " + url).c_str());
#endif
}

JNIEXPORT void JNICALL Java_ccfinderx_CCFinderX_openOfficialSiteTop
  (JNIEnv *env, jobject, jstring pageSuffix)
{
	const char* utf8PageSuffix = env->GetStringUTFChars(pageSuffix, NULL);
	std:: string str = utf8PageSuffix;
	std::string url = "http://www.ccfinder.net/index" + str + ".html";

	openUrl(url);

	env->ReleaseStringUTFChars(pageSuffix, utf8PageSuffix); 
}

JNIEXPORT void JNICALL Java_ccfinderx_CCFinderX_openOfficialSiteUserRgistrationPage
  (JNIEnv *env, jobject, jstring pageSuffix)
{
	const char* utf8PageSuffix = env->GetStringUTFChars(pageSuffix, NULL);
	std:: string str = utf8PageSuffix;
	std::string url = "http://www.ccfinder.net/userregistration" + str + ".html";

	openUrl(url);

	env->ReleaseStringUTFChars(pageSuffix, utf8PageSuffix); 
}

JNIEXPORT void JNICALL Java_ccfinderx_CCFinderX_openOfficialSiteDocumentPage
  (JNIEnv *env, jobject, jstring pageSuffix, jstring pageFileName)
{
	const char *p = getenv("CCFINDERX_DOCUMENT_URL");
	if (p != NULL) {
		openUrl(p);
		return;
	}
	
	const char* utf8PageSuffix = env->GetStringUTFChars(pageSuffix, NULL);
	std::string str = utf8PageSuffix;
	if (str.length() == 0) {
		str == "en";
	}
	const char* utf8PageFileName = env->GetStringUTFChars(pageFileName, NULL);
	std::string page  = utf8PageFileName;
	std::string url = "http://www.ccfinder.net/doc/10.2/" + str + "/" + page;

	openUrl(url);

	env->ReleaseStringUTFChars(pageFileName, utf8PageFileName); 
	env->ReleaseStringUTFChars(pageSuffix, utf8PageSuffix); 
}


int exec_ccfx(const std:: vector<std:: string> &argsUtf8)
{
	assert(!! oModuleDir);

#if defined OS_WIN32
	std:: string arg0 = *oModuleDir + file_separator() + "ccfx.exe";
#elif defined OS_UNIX
	std:: string arg0 = *oModuleDir + file_separator() + "ccfx";
#endif

	ArgvBuilder argv;
	argv.push_back(arg0);

	for (size_t i = 0; i < argsUtf8.size(); ++i) {
		const std::string &arg = INNER2SYS(argsUtf8[i]);
		argv.push_back(arg);
	}

	std:: cerr << "exec: " << argv.str() << std::endl;

#if defined _MSC_VER
	int r = _spawnv(_P_WAIT, arg0.c_str(), argv.c_argv());
	//int r;
	//{
	//	PROCESS_INFORMATION pi;
	//	STARTUPINFO si;
	//	::ZeroMemory(&si, sizeof(si));
	//	si.cb = sizeof(si);
	//	std::string s = argv.str();
	//	std::vector<char> buffer(s.begin(), s.end());
	//	buffer.push_back('\0');
	//	BOOL result = ::CreateProcess(NULL, &buffer[0], NULL, NULL, TRUE, 
	//		CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
	//	if (result == 0) {
	//		r = -1;
	//	}
	//	else {
	//		r = 0;
	//		::CloseHandle(pi.hThread);
	//		::WaitForSingleObject(pi.hProcess, INFINITE);
	//		DWORD exitCode;
	//		if (::GetExitCodeProcess(pi.hProcess, &exitCode)) {
	//			r = exitCode;
	//		}
	//		else {
	//			r = -1;
	//		}
	//		::CloseHandle(pi.hProcess);
	//	}
	//}
#elif defined __GNUC__
	int r = systemv(argv.value());
#endif

	return r;
}

JNIEXPORT jbyteArray JNICALL Java_ccfinderx_CCFinderX_openPrepFile
  (JNIEnv *env, jobject, jstring fileName0, jstring postfix0)
{
	std:: string fileName;
	{
		const char *str = env->GetStringUTFChars(fileName0, NULL);
		fileName = INNER2SYS(str);
		env->ReleaseStringUTFChars(fileName0, str);
	}
	std:: string postfix;
	{
		const char *str = env->GetStringUTFChars(postfix0, NULL);
		postfix = INNER2SYS(str);
		env->ReleaseStringUTFChars(postfix0, str);
	}

	std:: vector<unsigned char> buffer;

	bool fileRead = false;
	FILE *pf = fopen((fileName + postfix).c_str(), "rb");
	if (pf != NULL) {
		fseek(pf, 0, SEEK_END);
		long size = ftell(pf);
		fseek(pf, 0, SEEK_SET);

		buffer.resize(size);
		fread(&buffer[0], sizeof(char), size, pf);
		fclose(pf);

		fileRead = true;
	}


	// 2008/04/02
	//if (! fileRead) {
	//	fileRead = preprocessPackLookup.read(&buffer, fileName, postfix);
	//}

	if (! fileRead) {
		return NULL;
	}


	{
		size_t j = 0;
		for (size_t i = 0; i < buffer.size(); ++i) {
			unsigned char ch = buffer[i];
			if (ch == '\r') {
				if (i + 1 < buffer.size() && buffer[i + 1] == '\n') {
					++i;
					buffer[j] = '\n';
					++j;
				}
				else {
					buffer[j] = '\n';
					++j;
				}
			}
			else {
				buffer[j] = buffer[i];
				++j;
			}
		}
		buffer.resize(j);
	}
	
	jbyteArray ary = env->NewByteArray(buffer.size());
#if defined _MSC_VER
	const unsigned char *p = &buffer[0];
	env->SetByteArrayRegion(ary, 0, buffer.size(), reinterpret_cast<const jbyte *>(p));
#elif defined __GNUC__
	unsigned char *p = &buffer[0];
	env->SetByteArrayRegion(ary, 0, buffer.size(), reinterpret_cast<jbyte *>(p));
#endif

	return ary;
}

JNIEXPORT void JNICALL Java_ccfinderx_CCFinderX_clearPrepFileCacheState
  (JNIEnv *env, jobject)
{
	//preprocessPackLookup.clear(); // 2008/04/02
}


JNIEXPORT jstring JNICALL Java_ccfinderx_CCFinderX_getPythonInterpreterPath
  (JNIEnv *env, jobject)
{
	assert(!! oModuleDir);

	std::string thePythonInterpreterPath;
	const char *p = getenv("CCFINDERX_PYTHON_INTERPRETER_PATH");
	if (p != NULL) {
		thePythonInterpreterPath = p;
		if (thePythonInterpreterPath.length() > 0 && thePythonInterpreterPath[0] != '\"' && thePythonInterpreterPath.find(' ') != std::string::npos) {
			thePythonInterpreterPath = '\"' + thePythonInterpreterPath + '\"';
		}
	}
	else {
#if defined _MSC_VER
		thePythonInterpreterPath = "C:" "\\" "Python26" "\\" "python.exe";
#elif defined OS_UBUNTU
		thePythonInterpreterPath = "/usr/bin/python";
#endif
	}
	return env->NewStringUTF(thePythonInterpreterPath.c_str()); // may return null
}

JNIEXPORT jstring JNICALL Java_ccfinderx_CCFinderX_getApplicationDataPath
  (JNIEnv *env, jobject)
{
	return env->NewStringUTF(SYS2INNER(get_application_data_path()).c_str());
}

JNIEXPORT jint JNICALL Java_ccfinderx_CCFinderX_getCurrentProcessId
  (JNIEnv *, jobject)
{
	int pid = ::getpid();
	return (jint)pid;
}

bool process_is_alive(int processId);

JNIEXPORT jboolean JNICALL Java_ccfinderx_CCFinderX_isProcessAlive
  (JNIEnv *, jobject, jint processId)
{
    return process_is_alive(processId);
}

#if defined __GNUC__
boost::optional<std::vector<std::string> > read_lines(const std::string &fileName)
{
    using namespace std;
        
    ifstream is(fileName.c_str(), ios::binary);
    if (! is) return boost::optional<std::vector<std::string> >(); // fail
    
    vector<string> lines;
    string line;
    while (getline(is, line)) {
        lines.push_back(line);
    }
    
    return boost::optional<std::vector<std::string> >(lines);
}

bool process_is_alive(int processId)
{

    /* 
    References
      /linux/fs/proc/array.c 
      /linux-mon/linux_monitor-2.rc2/src/ps.c
    */
    
    using namespace std;
    namespace bal = boost::algorithm;
    
    DIR *dp = opendir("/proc");
    if (dp == NULL) return false; // unknown
    struct dirent *ep;
    while ((ep = readdir(dp)) != NULL) {
        try {
            int pid = boost::lexical_cast<int, string>(ep->d_name);
            string fileName = (boost::format("/proc/%d/status") % pid).str();
            boost::optional<vector<string> > pLines = read_lines(fileName);
            if (! pLines) return false; // fail
            vector<string> lines = *pLines;
            for (size_t i = 0; i < lines.size(); ++i) {
                string line = lines[i];
                if (bal::starts_with(line, "State:")) {
                    vector<string> fields;
                    bal::split(fields, line, bal::is_space());
                    //cout << "field = \"" << fields[1] << "\"" << endl;
                    string v = fields.at(1);
                    assert(v.length() == 1 && string("RSDTZX").find(v) != string::npos);
                    return string("RSDT").find(v) != string::npos;
                }
            }
        } catch (boost::bad_lexical_cast &e) {
        }
    }
    return false; // unknown
}
#endif // __GNUC__

#if defined _MSC_VER
bool process_is_alive(int processId)
{
    HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)processId);
    jboolean r = h != 0;
    CloseHandle(h);
    return r;
}
#endif // _MSC_VER
