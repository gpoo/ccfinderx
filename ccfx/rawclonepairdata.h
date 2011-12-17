#if ! defined RAWCLONEPAIRDATA_H
#define RAWCLONEPAIRDATA_H

#include <cstdio>
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <limits>
#include "../common/hash_set_includer.h"
#include "../common/hash_map_includer.h"

#include <boost/optional/optional.hpp>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>

#include "../common/ffuncrenamer.h"

#include "../threadqueue/threadqueue.h"
#include "../common/unportable.h" // import flip endian
#include "../common/utf8support.h"
#include "../common/filestructwrapper.h"

#include "ccfxconstants.h"

#if ! defined CODE_CONVERSION_SUPPORT
#error must define CODE_CONVERSION_SUPPORT
#endif

#if ! defined WSTRING_CONVERSION_SUPPORT
#error must define WSTRING_CONVERSION_SUPPORT
#endif

namespace {
	bool read_line(std::string *pLine, FILE *pFile)
	{
		std::string line;
		boost::array<char, 1024> buf;
		while (true) {
			if (fgets(&buf[0], buf.size(), pFile) == NULL) {
				return false; // eof
			}
			boost::array<char, 1024>::iterator p = std::find(buf.begin(), buf.end(), '\n');
			if (p != buf.end()) {
				line.append(buf.begin(), p);
				(*pLine).swap(line);
				return true;
			}
			else {
				p = std::find(buf.begin(), buf.end(), '\0');
				assert(p != buf.end());
				line.append(buf.begin(), p);
			}
		}
	}
	bool is_ascii_nocontrol(const std::string &line)
	{
		for (size_t i = 0; i < line.length(); ++i) {
			int ch = line[i];
			if (! (0x20 <= ch && ch <= 0x7f)) {
				return false;
			}
		}
		return true;
	}
	bool is_utf8_nocontrol(const std::string &line)
	{
		size_t pos = 0;
		while (pos < line.length()) {
			size_t nextPos = nextCharUTF8String(line, pos);
			if (nextPos > pos) {
				if (nextPos - pos == 1) {
					int ch = line[pos];
					if (ch < 0x20 || ch == 0x7f) {
						return false;
					}
				}
			}
			else {
				assert(false);
				return false;
			}
			pos = nextPos;
		}
		return true;
	}
	bool is_name(const std::string &str)
	{
		std::string::size_type p = 0;
		if (p == str.length()) {
			return false;
		}
		char ch = str[p];
		if (! (ch == '_' || 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z')) {
			return false;
		}
		++p;
		while (p != str.length()) {
			ch = str[p];
			if (! (ch == '_' || 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9')) {
				return false;
			}
			++p;
		}
		return true;
	}
};

namespace rawclonepair {

#if defined _MSC_VER
#pragma pack(push,1)
#endif

struct RawFileBeginEnd {
public:
	boost::uint32_t file; // >= 1
	boost::uint32_t begin; // >= 0
	boost::uint32_t end; // >= 0
public:
	RawFileBeginEnd(boost::int32_t file_, boost::int32_t begin_, boost::int32_t end_)
		: file(file_), begin(begin_), end(end_)
	{
	}
	RawFileBeginEnd()
		: file(0), begin(0), end(0)
	{
	}
	RawFileBeginEnd(const RawFileBeginEnd &right)
		: file(right.file), begin(right.begin), end(right.end)
	{
	}
public:
	void swap(RawFileBeginEnd &right)
	{
		std:: swap(this->file, right.file);
		std:: swap(this->begin, right.begin);
		std:: swap(this->end, right.end);
	}
public:
	bool operator==(const RawFileBeginEnd &right) const
	{
		if (file == right.file) {
			if (begin == right.begin) {
				if (end == right.end) {
					return true;
				}
			}
		}
		return false;
	}
	bool operator<(const RawFileBeginEnd &right) const
	{
		if (file < right.file) {
			return true;
		}
		else if (file == right.file) {
			if (begin < right.begin) {
				return true;
			}
			else if (begin == right.begin) {
				if (end < right.end) {
					return true;
				}
				else if (end == right.end) {
				}
			}
		}
		return false;
	}
};

struct RawClonePair {
public:
	RawFileBeginEnd left;
	RawFileBeginEnd right;
	boost::uint64_t reference;
public:
	RawClonePair(const RawFileBeginEnd &left_, const RawFileBeginEnd &right_, boost::uint64_t reference_)
		: left(left_), right(right_), reference(reference_)
	{
	}
	RawClonePair()
		: left(), right(), reference(0)
	{
	}
	RawClonePair(boost::int32_t lFile_, boost::int32_t lBegin_, boost::int32_t lEnd_, boost::int32_t rFile_, boost::int32_t rBegin_, boost::int32_t rEnd_, boost::uint64_t reference_)
		: left(lFile_, lBegin_, lEnd_), right(rFile_, rBegin_, rEnd_), reference(reference_)
	{
	}
	RawClonePair(const RawClonePair &b)
		: left(b.left), right(b.right), reference(b.reference)
	{
	}
public:
	bool operator==(const RawClonePair &b) const
	{
		const RawClonePair &a = *this;

		if (a.left.file == b.left.file) {
			if (a.right.file == b.right.file) {
				if (a.left.begin == b.left.begin) {
					if (a.left.end == b.left.end) {
						if (a.right.begin == b.right.begin) {
							if (a.right.end == b.right.end) {
								if (a.reference == b.reference) {
									return true;
								}
							}
						}
					}
				}
			}
		}

		return false;
	}
	bool operator<(const RawClonePair &b) const
	{
		const RawClonePair &a = *this;

		if (a.left.file < b.left.file) {
			return true;
		}
		else if (a.left.file == b.left.file) {
			if (a.right.file < b.right.file) {
				return true;
			}
			else if (a.right.file == b.right.file) {
				if (a.left.begin < b.left.begin) {
					return true;
				}
				else if (a.left.begin == b.left.begin) {
					if (a.left.end < b.left.end) {
						return true;
					}
					else if (a.left.end == b.left.end) {
						if (a.right.begin < b.right.begin) {
							return true;
						}
						else if (a.right.begin == b.right.begin) {
							if (a.right.end < b.right.end) {
								return true;
							}
							else if (a.right.end == b.right.end) {
								if (a.reference < b.reference) {
									return true;
								}
								else {
									if (a.reference == b.reference) {
										NULL;
									}
								}
							}
						}
					}
				}
			}
		}

		return false;
	}
};

#if defined _MSC_VER
#pragma pack(pop)
#endif

inline bool read_version(const std:: string &file, boost::int32_t version[3], std:: string *pErrorMessage)
{
	FileStructWrapper pFile(file, "rb");
	if (! (bool)pFile) {
		*pErrorMessage = (boost::format("can't open a file '%s'") % file).str();
		return false;
	}

	{
		const std:: string magicNumber = "ccfxraw0";
		std:: vector<char> buf;
		buf.resize(magicNumber.size());
		FREAD(&buf[0], sizeof(char), buf.size(), pFile);
		for (size_t i = 0; i < magicNumber.size(); ++i) {
			if (buf[i] != magicNumber[i]) {
				*pErrorMessage = "not a ccfx file";
				return false;
			}
		}
	}

	{
		FREAD(version, sizeof(boost::int32_t), 3, pFile);
		for (size_t i = 0; i < 3; ++i) {
			flip_endian(&version[i], sizeof(boost::int32_t));
		}
	}

	return true;
}

size_t fwrite_RawClonePair(const RawClonePair *ary, size_t count, FILE *pOutput);
size_t fread_RawClonePair(RawClonePair *ary, size_t count, FILE *pInput);

class AppVersionChecker {
private:
	std::vector<boost::int32_t> versionChecker;
protected:
	virtual ~AppVersionChecker()
	{
	}
public:
	AppVersionChecker(const AppVersionChecker &right)
		: versionChecker(right.versionChecker)
	{
	}
	AppVersionChecker(const std::vector<boost::int32_t> &versionChecker_)
	{
		assert(versionChecker_.size() <= 3);
		versionChecker = versionChecker_;
	}
	AppVersionChecker(boost::int32_t num1, boost::int32_t num2)
	{
		versionChecker.push_back(num1);
		versionChecker.push_back(num2);
	}
protected:
	template<typename fwdIt>
	bool checkVersion(fwdIt it) const
	{
		for (size_t i = 0; i < versionChecker.size(); ++i) {
			boost::int32_t v = *it;
			if (v != versionChecker[i]) {
				return false;
			}
			++it;
		}
		return true;
	}
};

class RawClonePairPrinter {
private:
	std:: ostream *pOutput;
	std:: string errorMessage;
	boost::int32_t version[3];
	std::vector<std::pair<std::string /* option name */, std::string /* comment */> > optionDefinitions; // used in version <= 10.1.X Not used version >= 10.2.
	Decoder defaultDecoder;

public:
	RawClonePairPrinter()
		: pOutput(&std:: cout)
	{
	}
	void attachOutput(std:: ostream *pOutput_)
	{
		pOutput = pOutput_;
	}
	void setOptionDefinitions(const std::vector<std::pair<std::string /* option name */, std::string /* comment */> > &optionDefinitions_)
	{
		optionDefinitions = optionDefinitions_;
	}
public:
	std:: string getErrorMessage() const
	{
		return errorMessage;
	}
	bool print(const std:: string &file)
	{
		errorMessage.clear();

		FileStructWrapper pFile(file, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pFile) {
			errorMessage = (boost::format("can't open a file '%s'") % file).str();
			return false;
		}

		if (! printVersion(pFile)) {
			return false;
		}

		// here, member variable "version" is assigned

		assert(version[0] == 10);

		switch (version[1]) {
		case 0:
			if (! printOptions_v0(pFile)) {
				return false;
			}
			break;
		case 1:
			if (! printOptions_v1(pFile)) {
				return false;
			}
			break;
		case 2:
			if (! printOptions_v2(pFile)) {
				return false;
			}
			break;
		default:
			assert(false);
			break;
		}

		if (! printPreprocessScript(pFile)) {
			return false;
		}
		
		switch (version[1]) {
		case 0:
		case 1:
			if (! printInputFiles(pFile)) {
				return false;
			}
			break;
		case 2:
			if (! printInputFiles_v2(pFile)) {
				return false;
			}
			break;
		default:
			assert(false);
			break;
		}
		
		switch (version[1]) {
		case 0:
		case 1:
			// nothing to do
			break;
		case 2:
			if (! printInputFileRemarks(pFile)) {
				return false;
			}
			break;
		default:
			assert(false);
			break;
		}

		if (! printClonePairs(pFile)) {
			return false;
		}
		
		switch (version[1]) {
		case 0:
		case 1:
			// nothing to do
			break;
		case 2:
			if (! printCloneSetRemarks(pFile)) {
				return false;
			}
			break;
		default:
			assert(false);
			break;
		}

		return true;
	}
private:
	bool printVersion(FILE *pFile)
	{
		{
			const std:: string magicNumber = "ccfxraw0";
			std:: vector<char> buf;
			buf.resize(magicNumber.size());
			FREAD(&buf[0], sizeof(char), buf.size(), pFile);
			for (size_t i = 0; i < magicNumber.size(); ++i) {
				if (buf[i] != magicNumber[i]) {
					errorMessage = "not a ccfx file";
					return false;
				}
			}
		}

		{
			FREAD(version, sizeof(boost::int32_t), 3, pFile);
			for (size_t i = 0; i < 3; ++i) {
				flip_endian(&version[i], sizeof(boost::int32_t));
			}
			(*pOutput) << "version: ccfx " << version[0] << "." << version[1] << "." << version[2] << std:: endl;
		}

		{
			std:: vector<char> buf;
			buf.resize(4);
			size_t count = FREAD(&buf[0], sizeof(char), buf.size(), pFile);
			if (count != 4) {
				errorMessage = "broken file";
				return false;
			}
			std:: string formatString(buf.begin(), buf.end());
			if (formatString == "pa:s") {
				(*pOutput) << "format: pair_single" << std:: endl;
			}
			else if (formatString == "pa:d") {
				(*pOutput) << "format: pair_diploid" << std:: endl;
			}
			else {
				errorMessage = "wrong format";
				return false;
				assert(false);
			}
		}

		return true;
	}
	bool printOptions_v0(FILE *pFile)
	{
		if (! (version[0] == 10 && version[1] == 0 && version[2] == 5)) {
			errorMessage = "version mismatch";
			return false;
		}

		boost::int32_t tl;
		FREAD(&tl, sizeof(boost::int32_t), 1, pFile);
		flip_endian(&tl, sizeof(boost::int32_t));
		(*pOutput) << "min_length: " << tl << std:: endl;
		return true;
	}
	bool printOptions_v1(FILE *pFile)
	{
		boost::int32_t count;
		FREAD(&count, sizeof(boost::int32_t), 1, pFile);
		flip_endian(&count, sizeof(boost::int32_t));
		if (count != 4) {
			errorMessage = "wrong format";
			return false;
			assert(false);
		}

		assert(count < 2000);

		for (size_t i = 0; i < (size_t)count; ++i) {
			boost::int32_t v;
			FREAD(&v, sizeof(boost::int32_t), 1, pFile);
			flip_endian(&v, sizeof(boost::int32_t));
			if (i < optionDefinitions.size()) {
				(*pOutput) << "option: " << optionDefinitions[i].first << " " << v << " // " << optionDefinitions[i].second << std:: endl;
			}
			else {
				(*pOutput) << "unknown_option" << ": " << v << std:: endl;
			}
		}
		return true;
	}
	bool printOptions_v2(FILE *pFile)
	{
		std::string line;
		while (true) {
			if (! read_line(&line, pFile)) {
				errorMessage = "invalid option field";
				return false;
			}
			if (line.length() == 0) {
				break; // while true
			}
			std::string::size_type p = line.find('\t');
			if (p == std::string::npos) {
				errorMessage = "invalid option field";
				return false;
			}
			std::string name = line.substr(0, p);
			std::string value = line.substr(p + 1);
			if (! is_name(name)) {
				errorMessage = "invalid option name";
				return false;
			}
			if (! is_utf8_nocontrol(value)) {
				errorMessage = "invalid option value";
				return false;
			}
			(*pOutput) << "option: -" << name << " " << defaultDecoder.encode(toWStringV(value)) << std::endl;
		}
		return true;
	}
	bool printPreprocessScript(FILE *pFile)
	{
		(*pOutput) << "preprocess_script: ";

		while (! feof(pFile)) {
			int ch = fgetc(pFile);
			if (ch == '\n') {
				(*pOutput) << std:: endl;
				return true;
			}
			(*pOutput) << (char)ch;
		}
		errorMessage = "broken file";
		return false;
	}
	bool printInputFiles(FILE *pFile)
	{
		(*pOutput) << "source_files {" << std:: endl;
		std:: string fileName;
		while (! feof(pFile)) {
			int ch = fgetc(pFile);
			if (ch != '\n') {
				fileName += (char)ch;
			}
			else {
				boost::int32_t id;
				FREAD(&id, sizeof(boost::int32_t), 1, pFile);
				flip_endian(&id, sizeof(boost::int32_t));
				boost::int32_t len;
				FREAD(&len, sizeof(boost::int32_t), 1, pFile);
				flip_endian(&len, sizeof(boost::int32_t));

				(*pOutput) << id << "\t" << defaultDecoder.encode(toWStringV(fileName)) << "\t" << len << std:: endl;
				
				fileName.clear();

				int ch2 = fgetc(pFile);
				if (ch2 == '\n') {
					(*pOutput) << "}" << std:: endl;
					return true;
				}
				else {
					fileName += ch2;
				}
			}
		}
		errorMessage = "broken source-file item";
		return false;
	}
	bool printInputFiles_v2(FILE *pFile)
	{
		bool success = printInputFiles(pFile);

		boost::int32_t id;
		FREAD(&id, sizeof(boost::int32_t), 1, pFile);
		flip_endian(&id, sizeof(boost::int32_t));
		boost::int32_t len;
		FREAD(&len, sizeof(boost::int32_t), 1, pFile);
		flip_endian(&len, sizeof(boost::int32_t));
		if (! (id == 0 && len == 0)) {
			errorMessage == "invaild source-file terminator";
			return false;
		}

		return true;
	}
	bool printInputFileRemarks(FILE *pFile)
	{
		Decoder defaultDecoder;

		(*pOutput) << "source_file_remarks {" << std::endl;

		boost::optional<int> lastFileID;
		std::string line;
		while (true) {
			if (! read_line(&line, pFile)) {
				errorMessage = "invalid source-file remark";
				return false;
			}
			boost::int32_t id;
			FREAD(&id, sizeof(boost::int32_t), 1, pFile);
			flip_endian(&id, sizeof(boost::int32_t));

			if (line.length() == 0) {
				if (id != 0) {
					errorMessage = "invalid source-file remark terminator";
					return false;
				}
				break; // while true
			}
			if (! is_utf8_nocontrol(line)) {
				errorMessage = "invalid source-file remark text";
				return false;
			}
			if (! (lastFileID && *lastFileID == id)) {
				(*pOutput) << id << std::endl;
				lastFileID = id;
			}

			(*pOutput) << ":" << "\t" << defaultDecoder.encode(toWStringV(line)) << std:: endl;
		}

		(*pOutput) << "}" << std::endl;
		return true;
	}
	bool printClonePairs(FILE *pFile)
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		(*pOutput) << "clone_pairs {" << std:: endl;
		while (true) {
			RawClonePair pd;
			int c = fread_RawClonePair(&pd, 1, pFile);
			if (c == 0) {
				errorMessage = "broken file";
				return false;
			}
			if (pd == terminator) {
				break; // while true
			}
			(*pOutput) << pd.reference << "\t";
			(*pOutput) << pd.left.file << "." << pd.left.begin << "-" << pd.left.end;
			(*pOutput) << "\t";
			(*pOutput) << pd.right.file << "." << pd.right.begin << "-" << pd.right.end;
			(*pOutput) << std:: endl;
		}
		(*pOutput) << "}" << std:: endl;

		return true;
	}
	bool printCloneSetRemarks(FILE *pFile)
	{
		Decoder defaultDecoder;

		(*pOutput) << "clone_set_remarks {" << std::endl;

		boost::optional<boost::int64_t> lastCloneSetID;
		std::string line;
		while (true) {
			if (! read_line(&line, pFile)) {
				errorMessage = "invalid clone-set remark";
				return false;
			}
			boost::int64_t id;
			FREAD(&id, sizeof(boost::int64_t), 1, pFile);
			flip_endian(&id, sizeof(boost::int64_t));

			if (line.length() == 0) {
				if (id != 0) {
					errorMessage = "invalid clone-set remark terminator";
					return false;
				}
				break; // while true
			}
			if (! is_utf8_nocontrol(line)) {
				errorMessage = "invalid clone-set remark text";
				return false;
			}
			if (! (lastCloneSetID && *lastCloneSetID == id)) {
				(*pOutput) << id << std::endl;
				lastCloneSetID = id;
			}

			(*pOutput) << ":" << "\t" << defaultDecoder.encode(toWStringV(line)) << std:: endl;
		}

		(*pOutput) << "}" << std::endl;
		return true;
	}
};

class RawClonePairFileTransformer : private AppVersionChecker {
public:
	struct RawFileData {
	public:
		int fileID;
		size_t length;
		std:: string path;
	public:
		RawFileData()
			: fileID(0), length(0), path()
		{
		}
		RawFileData(const RawFileData &right)
			: fileID(right.fileID), length(right.length), path(right.path)
		{
		}
		RawFileData(int fileID_, size_t length_, std::string path_)
			: fileID(fileID_), length(length_), path(path_)
		{
		}
	};
private:
	size_t maxMemoryUse;
	unsigned long long blockSize;
	unsigned long long bodySize;
	boost::int64_t bodyStartPos;
	boost::int64_t bodyEndPos;
	boost::int64_t outputBodyStartPos;
	boost::int64_t outputBodyEndPos;
	size_t sourceFiles;
	std:: vector<std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> > blocks;
	std:: string errorMessage;
	RawClonePair terminator;
public:
	RawClonePairFileTransformer()
		: AppVersionChecker(APPVERSION[0], APPVERSION[1]),
		maxMemoryUse(0), terminator(0, 0, 0, 0, 0, 0, 0)
	{
	}
public:
	std:: string getErrorMessage() const
	{
		return errorMessage;
	}
	void setMemoryUsageLimit(size_t maxMemoryUse_)
	{
		maxMemoryUse = maxMemoryUse_;
	}
	bool sort(const std:: string &sorted, const std:: string &unsorted)
	{
		errorMessage.clear();

		std:: string tempInput = make_temp_file_on_the_same_directory(sorted, "ccfxsorting1", ".tmp");
		std:: string tempOutput = make_temp_file_on_the_same_directory(sorted, "ccfxsorting2", ".tmp");
		
		if (maxMemoryUse == 0) {
			blockSize = (512 * 1024 * 1024 /* 512MB */) / sizeof(RawClonePair);
		}
		else {
			blockSize = maxMemoryUse / sizeof(RawClonePair);
		}
		if (blockSize < 1000) {
			blockSize = 1000;
		}

		if (! copyHeader(tempInput, unsorted)) { // assign bodyStartPos, oututBodyStartPos, sourceFiles
			return false;
		}
		if (! copyHeader(tempOutput, unsorted)) {
			return false;
		}
		
		if (! copySortBlocks(tempOutput, unsorted)) { // assign bodyEndPos, outputBodyEndPos, bodySize, blocks
			return false;
		}

		while (blocks.size() > 1) {
			tempOutput.swap(tempInput);
			if (! mergeBlocks(tempOutput, tempInput)) { // assign blocks
				return false;
			}
		}
		assert(blocks.size() == 1 && blocks[0].second == bodySize - 1/* for terminator */);

		if (! copyFooter(tempOutput, unsorted)) {
			return false;
		}

		remove(tempInput.c_str());
		remove(sorted.c_str());
		int r = rename(tempOutput.c_str(), sorted.c_str());
		if (r != 0) {
			errorMessage = (boost::format("can't create a file '%s'") % sorted).str();
			return false;
		}

		return true;
	}
public:
	class Filter {
	public:
		virtual ~Filter() { }
		virtual bool isValidFileID(int fileID) { return true; }
		virtual bool isValidCloneID(boost::uint64_t cloneID) { return true; }
		virtual bool isValidClonePair(const RawFileBeginEnd &left, const RawFileBeginEnd &right, boost::uint64_t cloneID) 
		{
			return isValidCloneID(cloneID) && isValidFileID(left.file), isValidFileID(right.file);
		}
		virtual void filterOptions(std::vector<std::pair<std::string/* name */, std::string/* value */> > *pOptions) { }
	};
	bool filter(const std:: string &filtered, const std:: string &original, Filter *pFilter)
	{
		errorMessage.clear();

		if (! filterHeader(filtered, original, pFilter)) {
			return false;
		}
		
		if (! filterBody(filtered, original, pFilter)) {
			return false;
		}

		if (! filterFooter(filtered, original, pFilter)) {
			return false;
		} 

		return true;
	}
public:
	class FilterFileByFile {
	public:
		virtual ~FilterFileByFile() { }
		virtual bool isValidFileID(int fileID) { return true; }
		virtual void transformFiles(std:: vector<RawFileData> *pFiles)
		{
			std:: vector<RawFileData> filtered;
			filtered.reserve((*pFiles).size());
			for (size_t i = 0; i < (*pFiles).size(); ++i) {
				const RawFileData &file = (*pFiles)[i];
				if (isValidFileID(file.fileID)) {
					filtered.push_back(file);
				}
			}
			(*pFiles).swap(filtered);
		}
		virtual bool isValidCloneID(boost::uint64_t cloneID) { return true; }
		virtual bool isValidClonePair(const RawFileBeginEnd &left, const RawFileBeginEnd &right, boost::uint64_t cloneID) 
		{
			return isValidCloneID(cloneID) && isValidFileID(left.file), isValidFileID(right.file);
		}
		virtual void transformPairs(std:: vector<RawClonePair> *pPairs)
		{
			std::vector<RawClonePair> filtered;
			filtered.reserve((*pPairs).size());
			for (size_t i = 0; i < (*pPairs).size(); ++i) {
				const RawClonePair &pair = (*pPairs)[i];
				if (isValidCloneID(pair.reference) && isValidFileID(pair.left.file) && isValidFileID(pair.right.file)
						&& isValidClonePair(pair.left, pair.right, pair.reference)) {
					filtered.push_back(pair);
				}
			}
			(*pPairs).swap(filtered);
		}
		virtual void filterOptions(std::vector<std::pair<std::string/* name */, std::string/* value */> > *pOptions) { }
	};
	bool filterFileByFile(const std:: string &filtered, const std:: string &original, FilterFileByFile *pFilter)
	{
		errorMessage.clear();

		if (! filterHeaderFileByFile(filtered, original, pFilter)) {
			return false;
		}
		
		if (! filterBodyFileByFile(filtered, original, pFilter)) {
			return false;
		}

		if (! filterFooterFileByFile(filtered, original, pFilter)) {
			return false;
		}

		return true;
	}
private:
	bool copyHeader(const std:: string &output, const std:: string &input)
	{
		FileStructWrapper pOutput(output, "wb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}

		if (! copyVersion(pOutput, pInput)) {
			return false;
		}

		if (! copyFormat(pOutput, pInput)) {
			return false;
		}
		
		if (! copyOptions_v2(pOutput, pInput)) {
			return false;
		}

		if (! copyPreprocessScript(pOutput, pInput)) {
			return false;
		}

		if (! copyInputFiles_v2(pOutput, pInput)) {
			return false;
		}

		if (! copyInputFileRemarks(pOutput, pInput)) {
			return false;
		}
		
		bodyStartPos = FTELL64(pInput);
		outputBodyStartPos = FTELL64(pOutput);
		assert(bodyStartPos == outputBodyStartPos);

		return true;
	}
	bool copySortBlocks(const std:: string &output, const std:: string &input)
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		assert(bodyStartPos == outputBodyStartPos);

		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FSEEK64(pOutput, outputBodyStartPos, SEEK_SET);
		
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}
		FSEEK64(pInput, bodyStartPos, SEEK_SET);
		
		std:: vector<RawClonePair> buffer;
		assert(blockSize < std::numeric_limits<size_t>::max());
		buffer.resize(blockSize);
		
		bodySize = 0;
		while (true) {
			std:: pair<boost::int64_t, unsigned long long> block;
			block.first = FTELL64(pInput);
			size_t readCount = fread_RawClonePair(&buffer[0], buffer.size(), pInput);
			
			bool includingTerminator = readCount > 0 && buffer[readCount - 1] == terminator;
			if (includingTerminator) {
				block.second = readCount - 1;
				blocks.push_back(block);
				std:: sort(buffer.begin(), buffer.begin() + readCount - 1);
				fwrite_RawClonePair(&buffer[0], readCount, pOutput);
				bodySize += readCount;
				break; // while
			}
			block.second = readCount;
			blocks.push_back(block);
			std:: sort(buffer.begin(), buffer.begin() + readCount);
			fwrite_RawClonePair(&buffer[0], readCount, pOutput);
			bodySize += readCount;
			if (readCount < buffer.size()) {
				assert(false);
				break;
			}
		}

		//bodyEndPos = FTELL64(pInput);
		bodyEndPos = bodyStartPos + sizeof(rawclonepair::RawClonePair) * bodySize;
		outputBodyEndPos = FTELL64(pOutput);
		assert(bodyEndPos == outputBodyEndPos);
		
		return true;
	}
	bool copyFooter(const std::string &output, const std::string &input)
	{
		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FSEEK64(pOutput, bodyEndPos, SEEK_SET);
		
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}
		FSEEK64(pInput, outputBodyEndPos, SEEK_SET);

		if (! copyCloneSetRemarks(pOutput, pInput)) {
			return false;
		}

		return true;
	}

	template<typename Filter>
	bool filterHeader_i(const std:: string &output, const std:: string &input, Filter *pFilter)
	{
		FileStructWrapper pOutput(output, "wb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}

		if (! copyVersion(pOutput, pInput)) {
			return false;
		}
		if (! copyFormat(pOutput, pInput)) {
			return false;
		}
		if (! filterOptions_v2(pOutput, pInput, pFilter)) {
			return false;
		}
		if (! copyPreprocessScript(pOutput, pInput)) {
			return false;
		}
		filterInputFiles_v2(pOutput, pInput, pFilter);
		if (! copyInputFileRemarks(pOutput, pInput)) {
			return false;
		}

		bodyStartPos = FTELL64(pInput);
		outputBodyStartPos = FTELL64(pOutput);

		return true;
	}
	bool filterHeader(const std:: string &output, const std:: string &input, Filter *pFilter)
	{
		return filterHeader_i<Filter>(output, input, pFilter);
	}
	bool filterHeaderFileByFile(const std:: string &output, const std:: string &input, FilterFileByFile *pFilter)
	{
		return filterHeader_i<FilterFileByFile>(output, input, pFilter);
	}

	bool filterBody(const std:: string &output, const std:: string &input, Filter *pFilter)
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FSEEK64(pOutput, outputBodyStartPos, SEEK_SET);
		
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}
		FSEEK64(pInput, bodyStartPos, SEEK_SET);
		
		bodySize = 0;
		while (true) {
			RawClonePair data;
			size_t readCount = fread_RawClonePair(&data, 1, pInput);
			if (readCount == 0) {
				errorMessage = "broken file";
				return false;
			}
			bodySize += readCount;
			if (data == terminator) {
				fwrite_RawClonePair(&data, readCount, pOutput);
				break; // while true
			}
			else {
				if ((*pFilter).isValidClonePair(data.left, data.right, data.reference)) {
					fwrite_RawClonePair(&data, readCount, pOutput);
				}
			}
		}

		//bodyEndPos = FTELL64(pInput);
		bodyEndPos = bodyStartPos + sizeof(rawclonepair::RawClonePair) * bodySize;
		outputBodyEndPos = FTELL64(pOutput);
		
		return true;
	}
	void transform_and_write(ThreadQueue<std::vector<RawClonePair> *> *pQue, FileStructWrapper *ppOutput, FilterFileByFile *pFilter)
	{
		FileStructWrapper &pOutput = *ppOutput;

		std::vector<RawClonePair> *pPairs;
		while ((pPairs = (*pQue).pop()) != NULL) {
			std::vector<RawClonePair> &pairs = *pPairs;
			
			(*pFilter).transformPairs(&pairs);
			
			if (! pairs.empty()) {
				boost::int32_t leftFile = pairs[0].left.file;

				if (pairs.front().left.file != leftFile) {
					errorMessage = "invalid transformation, file ID modified";
					return;
				}
				for (size_t i = 1; i < pairs.size(); ++i) {
					const RawClonePair &lastPair = pairs[i - 1];
					const RawClonePair &pair = pairs[i];
					if (pair.left.file != leftFile) {
						errorMessage = "invalid transformation, file ID modified";
						return;
					}
					if (! (lastPair < pair)) {
						errorMessage = "invalid transformation, pairs unsorted";
						return;
					}
				}
				fwrite_RawClonePair(&pairs[0], pairs.size(), pOutput);
			}

			delete pPairs;
		}
	}
	bool filterBodyFileByFile(const std:: string &output, const std:: string &input, FilterFileByFile *pFilter)
	{
		errorMessage = "";

		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FSEEK64(pOutput, outputBodyStartPos, SEEK_SET);
		
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}
		FSEEK64(pInput, bodyStartPos, SEEK_SET);
		
		ThreadQueue<std::vector<RawClonePair> *> que(10);
		boost::thread eater(boost::bind(&RawClonePairFileTransformer::transform_and_write, this, &que, &pOutput, pFilter));

		bodySize = 0;
		RawClonePair data;
		size_t readCount = fread_RawClonePair(&data, 1, pInput);
		if (readCount == 0) {
			errorMessage = "broken file";
			return false;
		}
		bodySize += readCount;
		while (true) {
			if (errorMessage != "") {
				return false;
			}

			if (data == terminator) {
				break; // while true
			}
			else {
				std::vector<RawClonePair> *pPairs = new std::vector<RawClonePair>();
				std::vector<RawClonePair> &pairs = *pPairs;
				pairs.push_back(data);
				boost::int32_t leftFile = pairs[0].left.file;
	
				while (true) {
					size_t readCount = fread_RawClonePair(&data, 1, pInput);
					if (readCount == 0) {
						errorMessage = "broken file";
						return false;
					}
					bodySize += readCount;
					if (data.left.file != leftFile || data == terminator) {
						break; // while
					}
					pairs.push_back(data);
				}
				assert(! pairs.empty());

				que.push(pPairs);
			}
		}

		que.push(NULL); // in order to terminate eater.
		eater.join();

		fwrite_RawClonePair(&data, readCount, pOutput);

		//bodyEndPos = FTELL64(pInput);
		bodyEndPos = bodyStartPos + sizeof(rawclonepair::RawClonePair) * bodySize;
		outputBodyEndPos = FTELL64(pOutput);
		
		return true;
	}
	
	template<typename Filter>
	bool filterFooter_i(const std:: string &output, const std:: string &input, Filter *pFilter)
	{
		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		FileStructWrapper pInput(input, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pInput) {
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}

		FSEEK64(pOutput, outputBodyEndPos, SEEK_SET);
		FSEEK64(pInput, bodyEndPos, SEEK_SET);

		if (! copyCloneSetRemarks(pOutput, pInput)) {
			return false;
		}

		return true;
	}
	bool filterFooter(const std:: string &output, const std:: string &input, Filter *pFilter)
	{
		return filterFooter_i<Filter>(output, input, pFilter);
	}
	bool filterFooterFileByFile(const std:: string &output, const std:: string &input, FilterFileByFile *pFilter)
	{
		return filterFooter_i<FilterFileByFile>(output, input, pFilter);
	}

	struct FRD {
	public:
		FILE *pFile;
		unsigned long long rest;
		RawClonePair curData;
	public:
		FRD(FILE *pFile_, unsigned long long rest_)
			: pFile(pFile_), rest(rest_), curData()
		{
		}
		FRD(const FRD &right)
			: pFile(right.pFile), rest(right.rest), curData(right.curData)
		{
		}
		FRD()
			: pFile(NULL), rest(0), curData()
		{
		}
		void swap(FRD &right)
		{
			std:: swap(pFile, right.pFile);
			std:: swap(rest, right.rest);
			std:: swap(curData, right.curData);
		}
	};
	bool mergeBlocks(const std:: string &output, const std:: string &input)
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		FileStructWrapper pOutput(output, "r+b" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pOutput) {
			return false;
		}
		assert(bodyStartPos == outputBodyStartPos);
		FSEEK64(pOutput, bodyStartPos, SEEK_SET);
		setvbuf(pOutput, NULL, _IOFBF, maxMemoryUse);

		std:: vector<std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> > newBlocks;

		size_t bi = 0;
		while (bi < blocks.size()) {
			std:: pair<boost::int64_t, unsigned long long> newBlock;
			newBlock.first = blocks[bi].first;
			newBlock.second = 0;

			std:: vector<FRD> inputs;
			size_t count;
			for (count = 0; count < 8 && bi + count < blocks.size(); ++count) {
				FILE *p = fopen(input.c_str(), "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
				std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> &b = blocks[bi + count];
				newBlock.second += b.second;
				FSEEK64(p, b.first, SEEK_SET);
				FRD frd(p, b.second);
				if (frd.rest != 0) {
					fread_RawClonePair(&frd.curData, 1, frd.pFile);
					assert(! (frd.curData == terminator));
				}
				inputs.push_back(frd);
			}
			
			while (true) {
				for (size_t i = 0; i < inputs.size(); ++i) {
					if (inputs[i].rest == 0) {
						fclose(inputs[i].pFile);
						inputs[i].swap(inputs[inputs.size() - 1]);
						inputs.pop_back();
					}
				}
				if (inputs.empty()) {
					break; // while true
				}

				size_t minIndex = 0;
				for (size_t i = 1; i < inputs.size(); ++i) {
					if (inputs[i].curData < inputs[minIndex].curData) {
						minIndex = i;
					}
				}
				
				FRD &frd = inputs[minIndex];
				fwrite_RawClonePair(&frd.curData, 1, pOutput);
				--frd.rest;
				if (frd.rest != 0) {
					fread_RawClonePair(&frd.curData, 1, frd.pFile);
					assert(! (frd.curData == terminator));
				}
			}

			newBlocks.push_back(newBlock);
			bi += count;
		}

		fwrite_RawClonePair(&terminator, 1, pOutput);

		blocks.swap(newBlocks);
		
		return true;
	}
	bool copyVersion(FILE *pOutput, FILE *pInput)
	{
		{
			const std:: string magicNumber = "ccfxraw0";
			std:: vector<char> buf;
			buf.resize(magicNumber.size());
			FREAD(&buf[0], sizeof(char), buf.size(), pInput);
			for (size_t i = 0; i < magicNumber.size(); ++i) {
				if (buf[i] != magicNumber[i]) {
					errorMessage = "not a ccfx file";
					return false;
				}
			}
			FWRITEBYTES(&buf[0], buf.size(), pOutput);
		}

		{
			boost::int32_t v[3];
			
			FREAD(v, sizeof(boost::int32_t), 3, pInput);
			for (size_t i = 0; i < 3; ++i) {
				flip_endian(&v[i], sizeof(boost::int32_t));
			}
			
			if (! checkVersion(&v[0])) {
				errorMessage = "version mismatch";
				return false;
			}

			for (size_t i = 0; i < 3; ++i) {
				flip_endian(&v[i], sizeof(boost::int32_t));
			}
			FWRITE(v, sizeof(boost::int32_t), 3, pOutput);
		}

		return true;
	}
	bool copyFormat(FILE *pOutput, FILE *pInput)
	{
		std:: vector<char> buf;
		buf.resize(4);
		size_t count = FREAD(&buf[0], sizeof(char), buf.size(), pInput);
		if (count != 4) {
			errorMessage = "broken file";
			return false;
		}
		std:: string formatString(buf.begin(), buf.end());
		//if (! (formatString == "pa:s" || formatString == "pa:d")) {
		//	errorMessage = "wrong format";
		//	return false;
		//}
		FWRITEBYTES(&buf[0], buf.size(), pOutput);

		return true;
	}
	//bool makeDiploidFormat(FILE *pOutput, FILE *pInput)
	//{
	//	std:: vector<char> buf;
	//	buf.resize(4);
	//	size_t count = FREAD(&buf[0], sizeof(char), buf.size(), pInput);
	//	if (count != 4) {
	//		errorMessage = "broken file";
	//		return false;
	//	}
	//	std:: string formatString(buf.begin(), buf.end());
	//	if (formatString != "pa:s") {
	//		errorMessage = "wrong format";
	//		return false;
	//	}
	//	const std:: string s = "pa:d";
	//	FWRITEBYTES(s.data(), s.length(), pOutput);
	//	
	//	return true;
	//}
	template<typename Filter>
	bool filterOptions_v2(FILE *pOutput, FILE *pInput, Filter *pFilter)
	{
		std::vector<std::pair<std::string/* name */, std::string/* value */> > table;

		std::string line;
		while (true) {
			if (! read_line(&line, pInput)) {
				errorMessage = "invalid option field";
				return false;
			}
			if (line.length() == 0) {
				break; // while true
			}
			std::string::size_type p = line.find('\t');
			if (p == std::string::npos) {
				errorMessage = "invalid option field";
				return false;
			}
			std::string name = line.substr(0, p);
			std::string value = line.substr(p + 1);
			if (! is_name(name)) {
				errorMessage = "invalid option name";
				return false;
			}
			if (! is_utf8_nocontrol(value)) {
				errorMessage = "invalid option value";
				return false;
			}
			table.push_back(std::pair<std::string, std::string>(name, value));
		}

		(*pFilter).filterOptions(&table);

		for (size_t i = 0; i < table.size(); ++i) {
			const std::string &name = table[i].first;
			const std::string &value = table[i].second;
			if (! is_name(name)) {
				errorMessage = "invalid option name generation";
				return false;
			}
			if (! is_utf8_nocontrol(value)) {
				errorMessage = "invalid option value generation";
				return false;
			}
			FWRITEBYTES(name.data(), name.length(), pOutput);
			FPUTC('\t', pOutput);
			FWRITEBYTES(value.data(), value.length(), pOutput);
			FPUTC('\n', pOutput);
		}
		FPUTC('\n', pOutput);

		return true;
	}
	//bool copyOptions(FILE *pOutput, FILE *pInput)
	//{
	//	boost::int32_t c;
	//	if (FREAD(&c, sizeof(boost::int32_t), 1, pInput) != 1) {
	//		errorMessage = "broken file";
	//		return false;
	//	}
	//	FWRITE(&c, sizeof(boost::int32_t), 1, pOutput);
	//	flip_endian(&c);

	//	for (size_t i = 0; i < c; ++i) {
	//		boost::int32_t value;
	//		if (FREAD(&value, sizeof(boost::int32_t), 1, pInput) != 1) {
	//			errorMessage = "broken file";
	//			return false;
	//		}
	//		FWRITE(&value, sizeof(boost::int32_t), 1, pOutput);
	//	}

	//	return true;
	//}
	bool copyOptions_v2(FILE *pOutput, FILE *pInput)
	{
		while (true) {
			std::string line;
			if (! read_line(&line, pInput)) {
				errorMessage = "broken file";
				return false;
			}
			if (line.length() == 0) {
				FPUTC('\n', pOutput);
				break; // while true
			}
			std::string::size_type pos = line.find('\t');
			if (pos == std::string::npos) {
				errorMessage = "invalid option";
				return false;
			}
			std::string name = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			if (! is_name(name)) {
				errorMessage = "invalid option name";
				return false;
			}
			if (! is_utf8_nocontrol(value)) {
				errorMessage = "invalid option text";
				return false;
			}
			FWRITEBYTES(line.data(), line.length(), pOutput);
			FPUTC('\n', pOutput);
		}
		return true;
	}
	bool copyPreprocessScript(FILE *pOutput, FILE *pInput)
	{
		while (! feof(pInput)) {
			int ch = fgetc(pInput);
			FPUTC(ch, pOutput);
			if (ch == '\n') {
				return true;
			}
		}
		errorMessage = "wrong format";
		return false;
	}
	bool copyInputFiles(FILE *pOutput, FILE *pInput)
	{
		sourceFiles = 0;
		while (! feof(pInput)) {
			int ch = fgetc(pInput);
			FPUTC(ch, pOutput);
			if (ch == '\n') {
				boost::int32_t id;
				FREAD(&id, sizeof(boost::int32_t), 1, pInput);
				FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);

				boost::int32_t len;
				FREAD(&len, sizeof(boost::int32_t), 1, pInput);
				FWRITE(&len, sizeof(boost::int32_t), 1, pOutput);

				++sourceFiles;
				int ch2 = fgetc(pInput);
				FPUTC(ch2, pOutput);
				if (ch2 == '\n') {
					return true;
				}
			}
		}
		errorMessage = "invalid source file";
		return false;
	}
	bool copyInputFiles_v2(FILE *pOutput, FILE *pInput)
	{
		if (! copyInputFiles(pOutput, pInput)) {
			return false;
		}

		boost::int32_t id;
		FREAD(&id, sizeof(boost::int32_t), 1, pInput);
		if (id != 0) {
			errorMessage = "invalid source-file terminator";
			return false;
		}
		FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);

		boost::int32_t len;
		FREAD(&len, sizeof(boost::int32_t), 1, pInput);
		if (len != 0) {
			errorMessage = "invalid source-file terminator";
			return false;
		}
		FWRITE(&len, sizeof(boost::int32_t), 1, pOutput);
		return true;
	}

	bool copyInputFileRemarks(FILE *pOutput, FILE *pInput) 
	{
		while (true) {
			std::string line;
			if (! read_line(&line, pInput)) {
				errorMessage = "broken file";
				return false;
			}
			if (line.length() == 0) {
				break; // while true
			}
			if (! is_utf8_nocontrol(line)) {
				errorMessage = "invalid source-file remark text";
				return false;
			}
			FWRITE(line.data(), sizeof(char), line.length(), pOutput);
			FPUTC('\n', pOutput);

			boost::int32_t id;
			FREAD(&id, sizeof(boost::int32_t), 1, pInput);
			FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
		}

		FPUTC('\n', pOutput);
		boost::int32_t id;
		FREAD(&id, sizeof(boost::int32_t), 1, pInput);
		if (id != 0) {
			errorMessage = "invalid source-file remark terminator";
			return false;
		}
		FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
		
		return true;
	}

	bool copyCloneSetRemarks(FILE *pOutput, FILE *pInput) 
	{
		while (true) {
			std::string line;
			if (! read_line(&line, pInput)) {
				errorMessage = "broken file";
				return false;
			}
			if (line.length() == 0) {
				break; // while true
			}
			if (! is_utf8_nocontrol(line)) {
				errorMessage = "invalid clone-set remark text";
				return false;
			}
			FWRITE(line.data(), sizeof(char), line.length(), pOutput);
			FPUTC('\n', pOutput);

			boost::int64_t id;
			FREAD(&id, sizeof(boost::int64_t), 1, pInput);
			FWRITE(&id, sizeof(boost::int64_t), 1, pOutput);
		}

		FPUTC('\n', pOutput);
		boost::int64_t id;
		FREAD(&id, sizeof(boost::int64_t), 1, pInput);
		if (id != 0) {
			errorMessage = "invalid clone-set remark terminator";
			return false;
		}
		FWRITE(&id, sizeof(boost::int64_t), 1, pOutput);
		
		return true;
	}

	template<typename Filter>
	void filterInputFiles_v2(FILE *pOutput, FILE *pInput, Filter *pFilter)
	{
		std:: string buf;
		while (! feof(pInput)) {
			int ch = fgetc(pInput);
			buf += ch;
			if (ch == '\n') {
				boost::int32_t id;
				FREAD(&id, sizeof(boost::int32_t), 1, pInput);
				flip_endian(&id, sizeof(boost::int32_t));

				boost::int32_t len;
				FREAD(&len, sizeof(boost::int32_t), 1, pInput);
				flip_endian(&len, sizeof(boost::int32_t));

				if ((*pFilter).isValidFileID(id)) {
					FWRITEBYTES(buf.data(), buf.length(), pOutput); // it will put out an '\n'-terminated string
					flip_endian(&id, sizeof(boost::int32_t));
					FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
					flip_endian(&len, sizeof(boost::int32_t));
					FWRITE(&len, sizeof(boost::int32_t), 1, pOutput);
				}
				buf.clear();
				
				int ch2 = fgetc(pInput);
				if (ch2 == '\n') {
					FPUTC(ch2, pOutput);

					boost::int32_t id;
					FREAD(&id, sizeof(boost::int32_t), 1, pInput);
					assert(id == 0);
					boost::int32_t len;
					FREAD(&len, sizeof(boost::int32_t), 1, pInput);
					assert(len == 0);

					FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
					FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);

					return;
				}
				else {
					buf += ch2;
				}
			}
		}
	}
	template<typename FilterFileByFile>
	void filterInputFilesFileByFile_v2(FILE *pOutput, FILE *pInput, FilterFileByFile *pFilter)
	{
		std:: vector<RawFileData> files;

		std:: string buf;
		while (! feof(pInput)) {
			int ch = fgetc(pInput);
			buf += ch;
			if (ch == '\n') {
				boost::int32_t id;
				FREAD(&id, sizeof(boost::int32_t), 1, pInput);
				flip_endian(&id, sizeof(boost::int32_t));

				boost::int32_t len;
				FREAD(&len, sizeof(boost::int32_t), 1, pInput);
				flip_endian(&len, sizeof(boost::int32_t));

				files.push_back(RawFileData(id, len, buf.substr(0, buf.length() - 1)));
				buf.clear();
				
				int ch2 = fgetc(pInput);
				if (ch2 == '\n') {
					boost::int32_t id;
					FREAD(&id, sizeof(boost::int32_t), 1, pInput);
					assert(id == 0);
					boost::int32_t len;
					FREAD(&len, sizeof(boost::int32_t), 1, pInput);
					assert(len == 0);

					break; // while
				}
				else {
					buf += ch2;
				}
			}
		}

		(*pFilter).transformFiles(&files);

		for (size_t i = 0; i < files.size(); ++i) {
			const RawFileData &file = files[i];
			FWRITEBYTES(file.path.data(), file.path.length(), pOutput);
			FPUTC('\n', pOutput);
			boost::int32_t id = file.fileID;
			flip_endian(&id, sizeof(boost::int32_t));
			FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
			boost::int32_t len = file.length;
			flip_endian(&len, sizeof(boost::int32_t));
			FWRITE(&len, sizeof(boost::int32_t), 1, pOutput);
		}
		FPUTC('\n', pOutput);
		{
			boost::int32_t id = 0;
			boost::int32_t len = 0;
			FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
			FWRITE(&id, sizeof(boost::int32_t), 1, pOutput);
		}
	}
};

//enum { 
//	OPTION_MINIMUM_CLONE_LENGTH, 
//	OPTION_SHAPER_LEVEL, 
//	OPTION_USE_PARAMETER_UNIFICATION, 
//	OPTION_MINIMUM_TOKEN_SET_SIZE,
//OPTION_SIZE
//};

class RawClonePairFileAccessor : private AppVersionChecker {
private:	
	class IDPathLen {
	public:
		int id;
		std:: string path;
		size_t len;
	public:
		IDPathLen()
			: id(0), path(), len(0)
		{
		}
		IDPathLen(const IDPathLen &right)
			: id(right.id), path(right.path), len(right.len)
		{
		}
		IDPathLen(int id_, const std:: string &path_, size_t len_)
			: id(id_), path(path_), len(len_)
		{
		}
	};
public:
	enum { FILEDATA = 1 << 0, CLONEDATA = 1 << 1, FILEREMARK = 1 << 2, CLONEREMARK = 1 << 3 };
private:
	boost::int32_t version[3];
	std:: string errorMessage;
	std:: string dataFilePath;
	FILE *pDataFile;
	std:: string temporaryFilePath;
	FILE *pCloneSetIDToFileID;
	std:: vector<IDPathLen> fileDescriptions; // fileID -> IDPathLen
	std:: vector<std:: pair<boost::int64_t, boost::int64_t> > fileClonePairLocations; // fileID -> pos
	size_t fileCount;
	size_t maxFileID;
	boost::uint64_t maxCloneSetID;
	std::vector<std::pair<std::string/* name */, std::string/* value */> > options;
	std:: string preprocessScript;
	HASH_MAP<boost::int32_t/* file id */, std::vector<std::string> > fileRemarks; // fileID -> remark[]

	bool useCache;
	mutable boost::optional<std::pair<int/* fileID */, std:: vector<RawClonePair> > > clonePairsCache;
public:
	RawClonePairFileAccessor()
		: AppVersionChecker(APPVERSION[0], APPVERSION[1]),
		pDataFile(NULL), pCloneSetIDToFileID(NULL), fileCount(0), maxFileID(0), maxCloneSetID(0), options(),
		useCache(false), clonePairsCache()
	{
	}
	virtual ~RawClonePairFileAccessor()
	{
		close();
	}
public:
	void setCacheUsage(bool cacheUsage)
	{
		useCache = cacheUsage;
	}
	bool getCacheUsage() const
	{
		return useCache;
	}
	std:: string getErrorMessage() const
	{
		return errorMessage;
	}
	std:: string getPreprocessScript() const
	{
		return preprocessScript;
	}
	void getVersion(boost::int32_t versionNumber[3]) const
	{
		for (size_t i = 0; i < 3; ++i) {
			versionNumber[i] = version[i];
		}
	}
	std::vector<std::string> getOptionValues(const std::string &name) const
	{
		std::vector<std::string> r;
		for (size_t i = 0; i < options.size(); ++i) {
			if (options[i].first == name) {
				r.push_back(options[i].second);
			}
		}
		return r;
	}
	void getOptions(std::vector<std::pair<std::string/* name */, std::string/* value */> > *pOptions)
	{
		(*pOptions) = options;
	}
	bool open(const std:: string &path, unsigned int requiredData)
	{
		clonePairsCache.reset();

		dataFilePath = path;
		pDataFile = fopen(dataFilePath.c_str(), "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (pDataFile == NULL) {
			errorMessage = std:: string("can't open file '") + dataFilePath + "'";
			return false;
		}
		temporaryFilePath = ::make_temp_file_on_the_same_directory(dataFilePath, "ccfxrawclonepairdata", ".tmp");
		pCloneSetIDToFileID = fopen(temporaryFilePath.c_str(), "wb+" F_TEMPORARY_FILE_OPTIMIZATION);
		if (pCloneSetIDToFileID == NULL) {
			errorMessage = "can't create temporary file";
			close();
			return false;
		}
		
		if (! readVersion(pDataFile)) {
			close();
			return false;
		}
		
		if (! readFormat(pDataFile)) {
			close();
			return false;
		}
		
		readOptions_v2(pDataFile);

		if (! readPreprocessScript(pDataFile)) {
			close();
			return false;
		}
		
		if ((requiredData & FILEDATA) != 0) {
			if (! readInputFiles_v2(pDataFile)) {
				close();
				return false;
			}
		}
		else {
			if (! readInputFiles_v2(pDataFile)) {
				close();
				return false;
			}
			fileDescriptions.clear();
		}
		
		if ((requiredData & FILEREMARK) != 0) {
			if (! readFileRemarks(pDataFile)) {
				close();
				return false;
			}
		}
		else {
			if (! readFileRemarks(pDataFile)) {
				close();
				return false;
			}
			fileRemarks.clear();
		}
		
		if ((requiredData & CLONEDATA) != 0) {
			if (! readClonePairs(pDataFile)) {
				close();
				return false;
			}
		}

		if ((requiredData & CLONEREMARK) != 0) {
			errorMessage = "not implemented yet";
			assert(false);
			return false;
		}
		
		return true;
	}
private:
	bool readVersion(FILE *pDataFile)
	{
		{
			const std:: string magicNumber = "ccfxraw0";
			std:: vector<char> buf;
			buf.resize(magicNumber.size());
			FREAD(&buf[0], sizeof(char), buf.size(), pDataFile);
			for (size_t i = 0; i < magicNumber.size(); ++i) {
				if (buf[i] != magicNumber[i]) {
					errorMessage = "not a ccfx file";
					return false;
				}
			}
		}

		{
			FREAD(version, sizeof(boost::int32_t), 3, pDataFile);
			for (size_t i = 0; i < 3; ++i) {
				flip_endian(&version[i], sizeof(boost::int32_t));
			}
			if (! checkVersion(&version[0])) {
				errorMessage = "version mismatch";
				return false;
			}
		}

		return true;
	}
	bool readFormat(FILE *pDataFile) 
	{
		std:: vector<char> buf;
		buf.resize(4);
		size_t count = FREAD(&buf[0], sizeof(char), buf.size(), pDataFile);
		if (count != 4) {
			errorMessage = "broken file";
			return false;
		}
		std:: string formatString(buf.begin(), buf.end());
		//if (formatString != "pa:d") {
		//	errorMessage = "wrong format";
		//	return false;
		//}
		return true;
	}
	bool readOptions_v2(FILE *pDataFile)
	{
		while (true) {
			std::string line;
			if (! read_line(&line, pDataFile)) {
				errorMessage = "broken file";
				return false;
			}
			if (line.length() == 0) {
				break; // while true
			}
			std::string::size_type pos = line.find('\t');
			if (pos == std::string::npos) {
				errorMessage = "invalid option";
				return false;
			}
			std::string name = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			if (! is_name(name)) {
				errorMessage = "invalid option name";
				return false;
			}
			if (! is_utf8_nocontrol(value)) {
				errorMessage = "invalid option text";
				return false;
			}
			options.push_back(std::pair<std::string, std::string>(name, value));
		}
		return true;
	}
	bool readPreprocessScript(FILE *pDataFile)
	{
		std:: string buf;
		while (! feof(pDataFile)) {
			int ch = fgetc(pDataFile);
			if (ch == '\n') {
				preprocessScript = buf;
				return true;
			}
			buf += (char)ch;
		}
		errorMessage = "broken file";
		return false;
	}
	bool readInputFiles_v2(FILE *pDataFile)
	{
		std:: string fileName;
		while (! feof(pDataFile)) {
			int ch = fgetc(pDataFile);
			if (ch != '\n') {
				fileName += (char)ch;
			}
			else {
				boost::int32_t id;
				FREAD(&id, sizeof(boost::uint32_t), 1, pDataFile);
				flip_endian(&id, sizeof(boost::uint32_t));
				boost::int32_t len;
				FREAD(&len, sizeof(boost::uint32_t), 1, pDataFile);
				flip_endian(&len, sizeof(boost::uint32_t));

				IDPathLen ipl((int)id, fileName, (size_t)len);

				// inflate table if needed
				if (! (id < fileDescriptions.size())) {
					IDPathLen dummy(-1, "", 0);
					maxFileID = id;
					fileDescriptions.resize(id + 1, dummy);
					++fileCount;
				}
				
				IDPathLen &it = fileDescriptions[id];
				if (it.id != -1) {
					errorMessage = "FileID confliction";
					return false;
				}
				it = ipl;
				
				fileName.clear();

				int ch2 = fgetc(pDataFile);
				if (ch2 == '\n') {
					boost::int32_t id;
					FREAD(&id, sizeof(boost::int32_t), 1, pDataFile);
					boost::int32_t len;
					FREAD(&len, sizeof(boost::int32_t), 1, pDataFile);
					if (! (id == 0 && len == 0)) {
						errorMessage = "invalid source file terminator";
						return false;
					}
					return true;
				}
				else {
					fileName += ch2;
				}
			}
		}
		errorMessage = "broken file";
		return false;
	}
	bool readFileRemarks(FILE *pDataFile)
	{
		std::string line;
		while (true) {
			if (! read_line(&line, pDataFile)) {
				errorMessage = "invalid source-file remark";
				return false;
			}
			boost::int32_t id;
			FREAD(&id, sizeof(boost::int32_t), 1, pDataFile);
			flip_endian(&id, sizeof(boost::int32_t));

			if (line.length() == 0) {
				if (id != 0) {
					errorMessage = "invalid source-file remark terminator";
					return false;
				}
				break; // while true
			}
			if (! is_utf8_nocontrol(line)) {
				errorMessage = "invalid source-file remark text";
				return false;
			}
			fileRemarks[id].push_back(line);
		}

		return true;
	}
	bool readClonePairs(FILE *pDataFile)
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);

		fileClonePairLocations.resize(maxFileID + 1);

		boost::uint64_t cloneSetIDToFileIDSize = 0;

		int leftID = -1;
		boost::int64_t pos = FTELL64(pDataFile);
		while (true) {
			RawClonePair pd;
			int c = fread_RawClonePair(&pd, 1, pDataFile);
			if (c == 0) {
				errorMessage = "broken file";
				return false;
			}
			boost::int64_t nextPos = pos + c * sizeof(RawClonePair);

			if (pd == terminator) {
				if (leftID != -1) {
					fileClonePairLocations[leftID].second = pos;
				}
				break; // while true
			}
			if (pd.left.file != leftID) {
				if (leftID != -1) {
                    fileClonePairLocations[leftID].second = pos;
				}
				if (! (pd.left.file <= maxFileID && fileDescriptions[pd.left.file].id != -1)) {
					errorMessage = "invalid FileID in clone description";
					return false;
				}
				if (! (pd.right.file <= maxFileID && fileDescriptions[pd.right.file].id != -1)) {
					errorMessage = "invalid FileID in clone description";
					return false;
				}
				fileClonePairLocations[pd.left.file].first = pos;

				leftID = pd.left.file;
			}

			boost::uint64_t cloneSetID = pd.reference;
			
			// inflate file if needed
			if (! (cloneSetID < cloneSetIDToFileIDSize)) {
				int value = -1;
				FSEEK64(pCloneSetIDToFileID, cloneSetIDToFileIDSize * sizeof(int), SEEK_SET);
				while (! (cloneSetID < cloneSetIDToFileIDSize)) {
					FWRITE(&value, sizeof(int), 1, pCloneSetIDToFileID);
					++cloneSetIDToFileIDSize;
				}
				assert(cloneSetID < cloneSetIDToFileIDSize);
			}
			
			FSEEK64(pCloneSetIDToFileID, cloneSetID * sizeof(int), SEEK_SET);
			int value = pd.left.file;
			FWRITE(&value, sizeof(int), 1, pCloneSetIDToFileID);

			if (maxCloneSetID < cloneSetID) {
				maxCloneSetID = cloneSetID;
			}

			pos = nextPos;
		}

		return true;
	}
public:
	void close()
	{
		if (pDataFile != NULL) {
			fclose(pDataFile);
			pDataFile = NULL;
		}
		if (pCloneSetIDToFileID != NULL) {
			fclose(pCloneSetIDToFileID);
			pCloneSetIDToFileID = NULL;
			remove(temporaryFilePath.c_str());
		}
		dataFilePath.clear();
		fileClonePairLocations.clear();
		fileCount = 0;
		maxFileID = 0;
		maxCloneSetID = 0;
		options.clear();
		preprocessScript.clear();
	}
	size_t getFileCount() const
	{
		return fileCount;
	}
	void getFiles(std:: vector<int> *pFileIDs) const
	{
		(*pFileIDs).clear();
		(*pFileIDs).reserve(fileDescriptions.size());

		for (size_t i = 0; i < fileDescriptions.size(); ++i) {
			const IDPathLen &ipl = fileDescriptions[i];
			if (ipl.id != -1) {
				(*pFileIDs).push_back(ipl.id);
			}
		}
	}
	void getFileDescription(int fileID, std:: string *pFilePath, size_t *pLength) const
	{
		assert(fileID >= 0);

		if (! (fileID < fileDescriptions.size())) {
			assert(false);
			return;
		}

		const IDPathLen &ipl = fileDescriptions[fileID];
		if (ipl.id == -1) {
			assert(false);
			if (pFilePath != NULL) {
				(*pFilePath).clear();
			}
			if (pLength != NULL) {
				*pLength = 0;
			}
		}
		else {
			if (pFilePath != NULL) {
				*pFilePath = ipl.path;
			}
			if (pLength != NULL) {
				*pLength = ipl.len;
			}
		}
	}
	void getFileRemark(int fileID, std::vector<std::string> *pRemarks) const
	{
		assert(pRemarks != NULL);

		HASH_MAP<boost::int32_t/* file id */, std::vector<std::string> >::const_iterator i = fileRemarks.find(fileID);
		if (i != fileRemarks.end()) {
			(*pRemarks) = i->second;
		}
		else {
			(*pRemarks).clear();
		}
	}
	void getFileIDsHavingRemark(std::vector<int> *pFileIDs) const
	{
		assert(pFileIDs != NULL);

		std::vector<int> fileIDs;
		fileIDs.reserve(fileRemarks.size());
		for (HASH_MAP<boost::int32_t/* file id */, std::vector<std::string> >::const_iterator i = fileRemarks.begin();
				i != fileRemarks.end(); ++i) {
			int fileID = i->first;
			fileIDs.push_back(fileID);
		}
		std::sort(fileIDs.begin(), fileIDs.end());
		(*pFileIDs).swap(fileIDs);
	}
	void getRawClonePairsOfFile(int fileID, std:: vector<RawClonePair> *pClonePairs) const
	{
		assert(fileID >= 0);

		if (useCache && clonePairsCache && (*clonePairsCache).first == fileID) {
			*pClonePairs = (*clonePairsCache).second;
			return;
		}

		if (! (fileID < fileDescriptions.size())) {
			assert(false);
			return;
		}

		(*pClonePairs).clear();

		const IDPathLen &ipl = fileDescriptions[fileID];
		if (ipl.id == -1) {
			assert(false);
		}
		else {
			//std:: vector<std:: pair<boost::int64_t, boost::int64_t> > fileClonePairLocations; // fileID -> pos
			const std:: pair<boost::int64_t, boost::int64_t> &loc = fileClonePairLocations[ipl.id];
			if (loc.second > loc.first) {
				boost::int64_t count64 = (loc.second - loc.first) / sizeof(RawClonePair);
				assert(count64 <= std::numeric_limits<size_t>::max());
				size_t count = count64;
				(*pClonePairs).resize(count);
				FSEEK64(pDataFile, loc.first, SEEK_SET);
				size_t readCount = fread_RawClonePair(&(*pClonePairs)[0], count, pDataFile);
				assert(readCount == count);
			}
		}

		if (useCache) {
			clonePairsCache = std::pair<int/* fileID */, std:: vector<RawClonePair> >();
			clonePairsCache->first = fileID;
			clonePairsCache->second = *pClonePairs;
		}
	}
	void getRawClonePairsOfFile(int fileID, std:: vector<RawClonePair> *pClonePairs, boost::uint64_t cloneSetID) const
	{
		std:: vector<RawClonePair> pairs;
		getRawClonePairsOfFile(fileID, &pairs);
		(*pClonePairs).clear();
		for (size_t i = 0; i < pairs.size(); ++i) {
			if (pairs[i].reference == cloneSetID) {
				(*pClonePairs).push_back(pairs[i]);
			}
		}
	}
	boost::uint64_t getMaxCloneSetID() const
	{
		return maxCloneSetID;
	}
	boost::uint64_t getCloneSetCount() const
	{
		boost::uint64_t id;
		if (! getFirstCloneSetID(&id)) {
			return 0;
		}

		boost::uint64_t count = 1;
		while (getNextCloneSetID(&id)) {
			++count;
		}
		return count;
	}
	bool cloneSetExists(boost::uint64_t cloneSetID) const
	{
		if (cloneSetID > maxCloneSetID) {
			return false;
		}
		FSEEK64(pCloneSetIDToFileID, cloneSetID * sizeof(int), SEEK_SET);
		int value;
		FREAD(&value, sizeof(int), 1, pCloneSetIDToFileID);
		if (value == -1) {
			return false;
		}

		return true;
	}
	bool getFirstCloneSetID(boost::uint64_t *pCloneSetID) const
	{
		boost::uint64_t i = 0;
		FSEEK64(pCloneSetIDToFileID, i * sizeof(int), SEEK_SET);
		while (i <= maxCloneSetID) {
			int value = -1;
			size_t c = FREAD(&value, sizeof(int), 1, pCloneSetIDToFileID);
			if (c != 1) {
				return false;
			}
			if (value != -1) {
				*pCloneSetID = i;
				return true;
			}
			++i;
		}
		return false;
	}
	bool getNextCloneSetID(boost::uint64_t *pCloneSetID) const
	{
		boost::uint64_t i = *pCloneSetID + 1;
		FSEEK64(pCloneSetIDToFileID, i * sizeof(int), SEEK_SET);
		while (i <= maxCloneSetID) {
			int value;
			FREAD(&value, sizeof(int), 1, pCloneSetIDToFileID);
			if (value != -1) {
				*pCloneSetID = i;
				return true;
			}
			++i;
		}
		return false;
	}
	void getRawClonePairsOfCloneSet(boost::uint64_t cloneSetID, std:: vector<RawClonePair> *pClonePairs) const
	{
		(*pClonePairs).clear();

		if (! (cloneSetID <= maxCloneSetID)) {
			return;
		}

		std:: vector<int> filesToBeSearched;

		FSEEK64(pCloneSetIDToFileID, cloneSetID * sizeof(int), SEEK_SET);
		int value;
		FREAD(&value, sizeof(int), 1, pCloneSetIDToFileID);
		filesToBeSearched.push_back(value);

		HASH_SET<int> filesSearched;
		while (! filesToBeSearched.empty()) {
			std:: vector<int> filesNewlyFound;
			for (std:: vector<int>::const_iterator i = filesToBeSearched.begin(); i != filesToBeSearched.end(); ++i) {
				int fileID = *i;
				std:: vector<RawClonePair> clonePairs;
				getRawClonePairsOfFile(fileID, &clonePairs, cloneSetID);
				for (size_t j = 0; j < clonePairs.size(); ++j) {
					const RawClonePair &pair = clonePairs[j];
					assert(pair.left.file == fileID);
					if (pair.right.file != fileID && filesSearched.find(pair.right.file) == filesSearched.end()) {
						std:: vector<int>::iterator it = std::lower_bound(filesNewlyFound.begin(), filesNewlyFound.end(), pair.right.file);
						if (it == filesNewlyFound.end() || *it != pair.right.file) {
							filesNewlyFound.insert(it, pair.right.file);
						}
					}
				}
				(*pClonePairs).insert((*pClonePairs).end(), clonePairs.begin(), clonePairs.end());
			}
			filesSearched.insert(filesToBeSearched.begin(), filesToBeSearched.end());
			filesToBeSearched.swap(filesNewlyFound);
		}

		std:: sort((*pClonePairs).begin(), (*pClonePairs).end());
	}
	void getCodeFragmentsOfCloneSet(boost::uint64_t cloneSetID, std:: vector<rawclonepair::RawFileBeginEnd> *pCodeFragments) const
	{
		std:: vector<rawclonepair::RawFileBeginEnd> &codeFragments = *pCodeFragments;
		codeFragments.clear();

		if (! (cloneSetID <= maxCloneSetID)) {
			return;
		}

		std:: vector<int> filesToBeSearched;

		FSEEK64(pCloneSetIDToFileID, cloneSetID * sizeof(int), SEEK_SET);
		int value;
		FREAD(&value, sizeof(int), 1, pCloneSetIDToFileID);
		filesToBeSearched.push_back(value);

		HASH_SET<int> filesSearched;
		while (! filesToBeSearched.empty()) {
			std:: vector<int> filesNewlyFound;
			for (std:: vector<int>::const_iterator i = filesToBeSearched.begin(); i != filesToBeSearched.end(); ++i) {
				int fileID = *i;
				std:: vector<RawClonePair> clonePairs;
				getRawClonePairsOfFile(fileID, &clonePairs, cloneSetID);
				for (size_t j = 0; j < clonePairs.size(); ++j) {
					const RawClonePair &pair = clonePairs[j];
					assert(pair.left.file == fileID);
					if (pair.right.file != fileID && filesSearched.find(pair.right.file) == filesSearched.end()) {
						std:: vector<int>::iterator it = std::lower_bound(filesNewlyFound.begin(), filesNewlyFound.end(), pair.right.file);
						if (it == filesNewlyFound.end() || *it != pair.right.file) {
							filesNewlyFound.insert(it, pair.right.file);
						}
					}
					if (codeFragments.size() >= 2) {
						if (! ((codeFragments[codeFragments.size() - 2] == pair.left) || codeFragments[codeFragments.size() - 1] == pair.left)) {
							codeFragments.push_back(pair.left);
						}
						if (! ((codeFragments[codeFragments.size() - 2] == pair.right) || codeFragments[codeFragments.size() - 1] == pair.right)) {
							codeFragments.push_back(pair.right);
						}
					}
					else {
						codeFragments.push_back(pair.left);
						codeFragments.push_back(pair.right);
					}
				}
			}
			filesSearched.insert(filesToBeSearched.begin(), filesToBeSearched.end());
			filesToBeSearched.swap(filesNewlyFound);
		}

		std:: sort(codeFragments.begin(), codeFragments.end());
		std:: vector<rawclonepair::RawFileBeginEnd>::iterator endi = std:: unique(codeFragments.begin(), codeFragments.end());
		codeFragments.resize(endi - codeFragments.begin());
	}
};

}; // namespece rawclonepair

#endif // RAWCLONEPAIRDATA_H
