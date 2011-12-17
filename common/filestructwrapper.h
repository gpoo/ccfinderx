#if ! defined FILESTRUCTWRAPPER_H
#define FILESTRUCTWRAPPER_H

#include <cstdio>
#include <cstdlib>

#include <string>

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

#include "unportable.h"

class FileStructWrapper : boost::noncopyable {
private:
	FILE *p;
	boost::optional<std::string> fileName;
	boost::optional<std::string> fileAccess;
public:
	inline ~FileStructWrapper()
	{
		close();
	}
	inline FileStructWrapper(FILE *p_)
		: p(p_), fileName(), fileAccess()
	{
	}
	FileStructWrapper(const std::string &fileName_, const std::string &fileAccess_) 
		: p(NULL), fileName(), fileAccess()
	{
		p = ::fopen(fileName_.c_str(), fileAccess_.c_str());
#if defined _MSC_VER
		if (p == NULL) {
			boost::optional<std::string> s = get_short_path_name(fileName_);
			if (s) {
				p = ::fopen((*s).c_str(), fileAccess_.c_str());
			}
		}
#endif
		if (p != NULL) {
			fileName = fileName_;
			fileAccess = fileAccess_;
		}
	}
	inline FileStructWrapper()
		: p(NULL), fileName(), fileAccess()
	{
	}
public:
	bool open(const std::string &fileName_, const std::string &fileAccess_)
	{
		close();

		p = ::fopen(fileName_.c_str(), fileAccess_.c_str());
#if defined _MSC_VER
		if (p == NULL) {
			boost::optional<std::string> s = get_short_path_name(fileName_);
			if (s) {
				p = ::fopen((*s).c_str(), fileAccess_.c_str());
			}
		}
#endif
		if (p == NULL) {
			return false;
		}

		fileName = fileName_;
		fileAccess = fileAccess_;
		
		return true;
	}
	void close()
	{
		if (p != NULL) {
			if (p == stdin || p == stdout || p == stderr) {
				// nothing to do
			}
			else {
				::fclose(p);
			}
		}
		p = NULL;
		{
			boost::optional<std::string> d;
			boost::swap(fileAccess, d);
		}

		// fileName will not cleared
	}
	inline operator bool() const
	{
		return p != NULL;
	}
	inline operator FILE *() const
	{
		return p;
	}
	inline void swap(FileStructWrapper &right)
	{
		std::swap(p, right.p);
		boost::swap(fileName, right.fileName);
		boost::swap(fileAccess, right.fileAccess);
	}
public:
	inline FILE *getFileStruct() const
	{
		return p;
	}
	inline boost::optional<std::string> name() const
	{
		return fileName;
	}
	inline boost::optional<std::string> access() const
	{
		return fileAccess;
	}
	inline void remove()
	{
		assert(!! fileName);
		close();
		::remove(fileName->c_str());
	}
};

#endif // FILESTRUCTWRAPPER_H
