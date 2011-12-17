#if ! defined UNPORTABLE_H__
#define UNPORTABLE_H__

#if defined OS_UBUNTU
#define OS_UNIX
#endif

#if defined OS_WIN32
#elif defined OS_UNIX
#else
#error "OS IS NOT SPECIFIED"
#endif

#if defined _MSC_VER
#elif defined __GNUC__
#else
#error "COMPILER IS NOT SPECIFIED"
#endif

#if ! (defined BIG_ENDIAN || defined LITTLE_ENDIAN)
#error "ENDIAN IS NOT SPECIFIED"
#endif

#if defined _MSC_VER
#include <windows.h>
#define my_sleep(x) Sleep(x)
#elif defined __GNUC__
#include <unistd.h>
#define my_sleep(x) usleep(x / 1000)
#endif

#include <string>
#include <set>
#include <vector>
#include <limits>

#include <boost/optional.hpp>

#if defined _MSC_VER
#undef max
#undef min
#endif

std::string escape_spaces(const std::string &str);

bool splitpath(const std:: string &path, std:: string *pDir, std:: string *pFname, std:: string *pExt);

std:: string make_temp_file_on_the_same_directory(
		const std:: string &filePath, const std:: string &base, const std:: string &ext);

std:: string make_filename_on_the_same_directory(
		const std:: string &fileName, const std:: string &targetPathFile); 

bool find_files(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &extensions, const std:: string &directory);
bool find_named_directories(std:: vector<std:: string> *pFiles, const std:: set<std:: string> &names, const std:: string &directory);

std::string get_application_data_path();

struct PathTime {
public:
	std:: string path;
	time_t mtime;
public:
	static bool getFileMTime(const std:: string &path, PathTime *pPathTime);
public:
	bool operator==(const PathTime &right) const
	{
		return path == right.path && mtime == right.mtime;
	}
	bool operator!=(const PathTime &right) const { return ! operator==(right); }
};

#if defined _MSC_VER

class MappedFileReader {
private:
	std:: string fileName;
	HANDLE hMapping;
	LARGE_INTEGER size;
	const char *aByte;
	bool opened;
public: 
	MappedFileReader()
		: hMapping(NULL), size(), aByte(NULL), opened(false)
	{
	}
	~MappedFileReader()
	{
		close();
	}
	void swap(MappedFileReader &right)
	{
		fileName.swap(right.fileName);
		std:: swap(hMapping, right.hMapping);
		std:: swap(size, right.size);
		std:: swap(aByte, right.aByte);
		std:: swap(opened, right.opened);
	}
	bool open(const std:: string &fileName_)
	{
		if (opened) {
			close();
		}

		fileName = fileName_;
		opened = false;

		HANDLE hFile = ::CreateFile(
				fileName.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
				);

		if (hFile == 0) {
			return false; // fail
		}

		if (! ::GetFileSizeEx(hFile, &size)) {
			return false; // fail
		}
		
		hMapping = ::CreateFileMapping(
				hFile,
				NULL,
				PAGE_READWRITE,
				0,
				0,
				NULL
				);

		::CloseHandle(hFile);

		aByte = (const char *)::MapViewOfFile(
				hMapping,
				FILE_MAP_READ | FILE_MAP_WRITE,
				0,
				0,
				0
				);
		
		opened = true;
		
		return true;
	}
	void close()
	{
		if (opened) {
			opened = false;

			::CloseHandle(hMapping);
			hMapping = NULL;

			::UnmapViewOfFile(aByte);
			aByte = NULL;
			
			fileName.clear();
		}
	}
	size_t getSize() const
	{
		if (opened) {
			assert(size.QuadPart <= std::numeric_limits<size_t>::max());
			return (size_t)size.QuadPart;
		}
		else {
			return 0;
		}
	}
	const char *ref() const
	{
		if (opened) {
			return aByte;
		}
		else {
			return NULL;
		}
	}
	bool isOpened() const
	{
		return opened;
	}
	const std:: string getFileName() const
	{
		return fileName;
	}
};

#elif defined __GNUC__

#endif

std::string file_separator();
bool path_is_relative(const std::string &path);
//void split_path(const std::string &path, std::string *pDirectory, std::string *pFileName);
std::string join_path(const std::string &s1, const std::string &s2);
bool path_exists(const std::string &path);
bool path_is_file(const std::string &path);

//template <typename IntegerType>
//void flip_endian(IntegerType *pValue)
//{
//	flip_endian((void *)pValue, (size_t)sizeof(IntegerType));
//}

void flip_endian(void *pIntegerValue, size_t sizeofIntegerType);

#if defined _MSC_VER

void nice(int vaule); // make own process priority low, when value > 10.
bool allocWorkingSetMemory(long long size);

#else

#define nice(value)
#define allocWorkingSetMemory(size)

#endif

#if defined _MSC_VER

#define ALLOC_TRICK_IN_CLASS_DECLARATION(className) \
private: \
	static HANDLE s_hHeap; \
	static UINT s_uNumAllocsInHeap; \
public: \
	void *operator new(size_t size); \
	void operator delete(void *p); \
private:

#define ALLOC_TRICK_DEFINITIONS(className) \
HANDLE className::s_hHeap = NULL; \
UINT className::s_uNumAllocsInHeap = 0; \
 \
void *className::operator new(size_t size) \
{ \
	if (s_hHeap == NULL) { \
		s_hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0); \
	 \
		if (s_hHeap == NULL) { \
			return NULL; \
		} \
	} \
	 \
	void *p = HeapAlloc(s_hHeap, 0, size); \
	 \
	if (p != NULL) { \
		s_uNumAllocsInHeap++; \
	} \
	 \
	return p; \
} \
 \
void className::operator delete(void *p) \
{ \
	if (HeapFree(s_hHeap, 0, p)) { \
		--s_uNumAllocsInHeap; \
	} \
	 \
	if (s_uNumAllocsInHeap == 0) { \
		if (HeapDestroy(s_hHeap)) { \
			s_hHeap = NULL; \
		} \
	} \
}

#else

#define ALLOC_TRICK_IN_CLASS_DECL(className)

#define ALLOC_TRICK_DEFINITIONS(className)

#endif

#if defined _MSC_VER
#define F_SEQUENTIAL_ACCESS_OPTIMIZATION "S"
#define F_TEMPORARY_FILE_OPTIMIZATION "T"
#else
#define F_SEQUENTIAL_ACCESS_OPTIMIZATION
#define F_TEMPORARY_FILE_OPTIMIZATION
#endif

#if defined __GNUC__

int systemv(const std::vector<std::string> &argv);

#endif

boost::optional<std::string> getenvironmentvariable(const std::string &name);

#if defined _MSC_VER

boost::optional<std::string> get_short_path_name(const std::string &path);

#endif


#endif // UNPORTABLE_H__



