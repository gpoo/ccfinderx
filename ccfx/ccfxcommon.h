#if ! defined CCFXCOMMON_H
#define CCFXCOMMON_H

#include <ostream>
#include <string>
#include <cassert>
#include <fstream>
#include <vector>
#include <set>
#include <limits>

#include <boost/dynamic_bitset.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>

#if defined _MSC_VER
#undef max
#endif

#include "../common/hash_map_includer.h"
#include "../common/specialstringmap.h"
//#include "../../easyzip/zipfile.h" //2008/04/02

//extern const boost::int32_t APPVERSION[3];
//extern const boost::int32_t APPVERSION_FIX;
//extern const std::string PLATFORM_NAME;

extern const std::string PREPROCESSED_FILE_POSTFIX;

std:: string SYS2INNER(const std::string &systemString);
std:: string INNER2SYS(const std::string &innerString);

#if defined CCFX_TOKEN_32BIT
typedef boost::int32_t ccfx_token_t;
#else
typedef boost::int16_t ccfx_token_t;
#endif

inline ccfx_token_t remove_displacement(ccfx_token_t token)
{
	return token < 0 ? -1 : token;
}

template<typename Iterator>
inline void remove_displacement(Iterator begin, Iterator end) 
{
	for (Iterator i = begin; i < end; ++i) {
		*i = remove_displacement(*i);
	}
}

inline ccfx_token_t to_displacement(size_t currentPos, size_t referencePos)
{
	assert(referencePos < currentPos);

	size_t distance = currentPos - referencePos;
	if (distance >= std::numeric_limits<ccfx_token_t>::max()) {
		return -1;
	}

	return -(ccfx_token_t)(currentPos - referencePos) - 1; // <= -2
}

inline size_t to_reference_position(size_t currentPos, ccfx_token_t token)
{
	assert(token <= -2);

	size_t pos = currentPos;
	pos += token + 1;

	assert(pos < currentPos);

	return pos;
}

inline ccfx_token_t to_compared(const std::vector<ccfx_token_t> &seq, size_t pos, size_t begin)
{
	ccfx_token_t token = seq[pos];

	if (token >= -1) { // normal token or opened parameter token
		return token;
	}

	assert(token <= -2);

	size_t referencePos = to_reference_position(pos, token);
	if (referencePos < begin) {
		return -1; // opened parameter token
	}

	return token;
}

inline ccfx_token_t to_reversereference_compared(const std::vector<ccfx_token_t> &seq, size_t pos, size_t begin, size_t end)
{
	assert(pos < begin); // precondition

	if (seq[pos] >= 0) { // normal token
		return seq[pos];
	}

	for (size_t p = begin; p < end; ++p) {
		ccfx_token_t t = seq[p];
		if (t <= -2) {
			size_t referencePos = to_reference_position(p, t);
			if (referencePos == pos) {
				return to_displacement(p, pos);
			}
		}
	}
	return -1; // opened parameter token
}

extern const char PARAMETER_SEPARATOR;

extern std:: vector<unsigned char> licenseData;

extern std:: string theArgv0;
extern boost::optional<std::string> theTemporaryFileBaseName;
extern int theWaitAfterProcessInvocation;

extern std:: string thePythonInterpreterPath;

extern int theMaxWorkerThreads;

extern const std:: string prepExtension;
extern const std:: string pairBinaryDiploidExtension;

void force_extension(std:: string *pFileName, const std:: string &extension);
void force_extension(std:: string *pFileName, const std:: string &extension1, const std:: string &extension2);
void force_extension(std:: string *pFileName, const std:: string &extension1, const std:: string &extension2, const std:: string &extension3);
void force_extension(std:: string *pFileName, const std:: string *extensionBegin, const std:: string *extensionEnd);

//long long strtoll(const char *str, char **p, int radix);
bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines);

int read_script_table(std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > *pOutput, const std::string &argv0, 
		const boost::optional<std::vector<std::string> > &oOptionalPrepScriptDescriptionFiles);

inline int read_script_table(std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > *pOutput, const std::string &argv0)
{
	return read_script_table(pOutput, argv0, boost::optional<std::vector<std::string> >());
}

struct InputFileData {
public:
	int fileID;
	int groupID;
	std::string path;
public:
	InputFileData(int fileID_, int groupID_, const std::string &path_)
		: fileID(fileID_), groupID(groupID_), path(path_)
	{
	}
	InputFileData(const InputFileData &right)
		: fileID(right.fileID), groupID(right.groupID), path(right.path)
	{
	}
	InputFileData()
		: fileID(0), groupID(0), path()
	{
	}
public:
	inline int getFileID() const { return fileID; }
	inline int getGroupID() const { return groupID; }
	inline std::string getPath() const { return path; }
public:
	void swap(InputFileData &right)
	{
		std::swap(fileID, right.fileID);
		std::swap(groupID, right.groupID);
		path.swap(right.path);
	}
public:
	bool operator==(const InputFileData &right) const
	{
		return fileID == right.fileID && groupID == right.groupID && path == right.path;
	}
	bool operator<(const InputFileData &right) const
	{
		if (fileID < right.fileID) {
			return true;
		}
		else if (fileID == right.fileID) {
			if (groupID < right.groupID) {
				return true;
			}
			else if (groupID == right.groupID) {
				if (path < right.path) {
					return true;
				}
			}
		}
		return false;
	}
	bool operator!=(const InputFileData &right) const { return ! operator==(right); }
	bool operator>(const InputFileData &right) const { return right.operator<(*this); }
};

//class PreprocessPackLookup : boost::noncopyable {
//private:
//	struct ZipFileAndFilename {
//		zipfile::ZipFile zf;
//		std::vector<std::string> filenames;
//	};
//private:
//	mutable std::map<std::string/* postfix */, std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> > > preprocessPackLookup;
//public:
//	bool read(std::vector<unsigned char> *pBuffer, const std::string &filename, const std::string &postfix) const;
//	void clear();
//	void swap(PreprocessPackLookup &rhs);
//};

class PreprocessedFileRawReader {
private:
	std::vector<std::string> prepDirs;
	mutable std::vector<std::string> prepDirsWithoutPathSeparator;

public:
	void swap(PreprocessedFileRawReader &right)
	{
		prepDirs.swap(right.prepDirs);
		prepDirsWithoutPathSeparator.swap(right.prepDirsWithoutPathSeparator);
	}
	void clear()
	{
		prepDirs.clear();
		prepDirsWithoutPathSeparator.clear();
	}
	void addPreprocessFileDirectory(const std::string &path)
	{
		prepDirs.push_back(path);
	}
	void setPreprocessFileDirectories(const std::vector<std::string> &paths)
	{
		prepDirs.assign(paths.begin(), paths.end());
	}
	std::vector<std::string> getPreprocessFileDirectories() const
	{
		return prepDirs;
	}
	bool readLines(const std::string &fileName, const std::string &postfix, std::vector<std::string> *pLines) const;
};

class PreprocessedFileReader {
private:
	static const std::string PREFIX; // = "prefix:"
	static const std::string SUFFIX; // = "suffix:"
private:
	std::vector<std::string> parameterHeaddings;
	PreprocessedFileRawReader rawReader; 
	special_string_map<ccfx_token_t> codeTable;
	std::vector<std:: pair<ccfx_token_t, ccfx_token_t> > parens;
	std::vector<ccfx_token_t> prefixes;
	std::vector<ccfx_token_t> suffixes;
	HASH_MAP<std::string, size_t /* index */> parenNameToIndex;
	HASH_MAP<std::string, size_t /* index */> prefixNameToIndex;
	HASH_MAP<std::string, size_t /* index */> suffixNameToIndex;
	bool useParameterization;

private:
	boost::optional<std::string> normalizeNonParameterToken(const std::string &token) const
	{
		std::string::size_type p = token.find(PARAMETER_SEPARATOR);
		if (p != std:: string::npos) {
			for (size_t i = 0; i < parameterHeaddings.size(); ++i) {
				if (boost::algorithm::starts_with(token, parameterHeaddings[i])) {
					// token is a parameter token
					if (useParameterization) {
						return boost::optional<std::string>();
					}
					else {
						// a parameter token is treated as one of other kind of tokens.
						return token;
					}
				}
			}

			// a literal token
			return token.substr(0, p);
		}
		
		// others
		return token;
	}
	ccfx_token_t allocCode(const std:: string &token)
	{
		const special_string_map<ccfx_token_t> &ctbl = codeTable;
		special_string_map<ccfx_token_t> ::const_iterator i = ctbl.find(token);
		if (i != ctbl.end()) {
			return i->second;
		}
		assert(codeTable.size() < std::numeric_limits<ccfx_token_t>::max());
		ccfx_token_t newCode = (ccfx_token_t)codeTable.size();
		codeTable[token] = newCode;
		if (token.length() >= 1 && (token[0] == '(' || token[0] == ')')) {
			std:: string parenName = token.substr(1);
			HASH_MAP<std:: string, size_t /* index */>::iterator j = parenNameToIndex.find(parenName);
			if (j == parenNameToIndex.end()) {
				size_t index = parens.size();
				parenNameToIndex[parenName] = index;
				parens.push_back(std:: pair<ccfx_token_t, ccfx_token_t>(newCode, newCode));
			}
			else {
				size_t index = j->second;
				if (token[0] == '(') {
					parens[index].first = newCode;
				}
				else {
					parens[index].second = newCode;
				}
			}
		}
		else if (boost::starts_with(token, PREFIX)) {
			std::string prefixName = token.substr(PREFIX.length());
			HASH_MAP<std::string, size_t /* index */>::iterator j = prefixNameToIndex.find(prefixName);
			if (j == prefixNameToIndex.end()) {
				size_t index = prefixes.size();
				prefixNameToIndex[prefixName]= index;
				prefixes.push_back(newCode);
			}
		}
		else if (boost::starts_with(token, SUFFIX)) {
			std::string suffixName = token.substr(SUFFIX.length());
			HASH_MAP<std::string, size_t /* index */>::iterator j = suffixNameToIndex.find(suffixName);
			if (j == suffixNameToIndex.end()) {
				size_t index = suffixes.size();
				suffixNameToIndex[suffixName]= index;
				suffixes.push_back(newCode);
			}
		}
		return newCode;
	}
public:
	//void clear()
	//{
	//	rawReader.clear();
	//	codeTable.clear();
	//	codeTable["eof"] = 0;
	//	parens.clear();
	//	parenNameToIndex.clear();
	//}
	void swap(PreprocessedFileReader &right)
	{
		rawReader.swap(right.rawReader);
		PreprocessedFileReader &left = *this;
		left.codeTable.swap(right.codeTable);
		left.parens.swap(right.parens);
		left.prefixes.swap(right.prefixes);
		left.suffixes.swap(right.suffixes);
		left.parenNameToIndex.swap(right.parenNameToIndex);
		left.prefixNameToIndex.swap(right.prefixNameToIndex);
		left.suffixNameToIndex.swap(right.suffixNameToIndex);
	}
	PreprocessedFileReader()
		: useParameterization(true)
	{
		codeTable.reserve_length(40);
		codeTable["eof"] = 0;

		parameterHeaddings.push_back(std::string("id") + PARAMETER_SEPARATOR);
		parameterHeaddings.push_back(std::string("word") + PARAMETER_SEPARATOR);
	}
	PreprocessedFileReader(const PreprocessedFileReader &right)
		: rawReader(right.rawReader), codeTable(right.codeTable), parens(right.parens),
		prefixes(right.prefixes), suffixes(right.suffixes), parenNameToIndex(right.parenNameToIndex),
		prefixNameToIndex(right.prefixNameToIndex), suffixNameToIndex(right.suffixNameToIndex),
		useParameterization(right.useParameterization)
	{
	}
	void setRawReader(const PreprocessedFileRawReader &rawReader_)
	{
		rawReader = rawReader_;
	}
	void setParameterizationUsage(bool value)
	{
		this->useParameterization = value;
	}
	ccfx_token_t getCode(const std:: string &token) const
	{
		boost::optional<std::string> normalizedToken = normalizeNonParameterToken(token);
		if (! normalizedToken) {
			// is a parameter token
			return -1;
		}

		special_string_map<ccfx_token_t> ::const_iterator i = codeTable.find(*normalizedToken);
		if (i != codeTable.end()) {
			return i->second;
		}
		else {
			return 0; // value 0 is a code for token "eof", but the value is also used for error, the token has not allocated any code.
		}
	}
	void getTokenStrings(std:: set<std:: string> *pTokenStrings) const
	{
		(*pTokenStrings).clear();
		for (special_string_map<ccfx_token_t>::const_iterator i = codeTable.begin(); i != codeTable.end(); ++i) {
			(*pTokenStrings).insert(i->first);
		}
	}
	const std:: vector<std:: pair<ccfx_token_t, ccfx_token_t> > &refParens() const
	{
		return parens;
	}
	const std::vector<ccfx_token_t> &refPrefixes() const
	{
		return prefixes;
	}
	const std::vector<ccfx_token_t> &refSuffixes() const
	{
		return suffixes;
	}
public:
	bool readFile(const std:: string &fileName, const std::string &postfix, std:: vector<ccfx_token_t> *pSeq);
	bool readFileByName(const std:: string &fileName, std:: vector<ccfx_token_t> *pSeq);
	bool countLinesOfFile(const std:: string &fileName, const std::string &postfix,
		size_t *pLoc, // loc including comments or whitespaces
		size_t *pSloc, // count of lines that contains tokens (comments will be excluded)
		size_t *pLocOfAvailableTokens, // count of lines that contains specified tokens by availableTokens
		const boost::dynamic_bitset<> *pAvailableTokens);
};

bool getPreprocessedSequenceOfFile(std:: vector<ccfx_token_t> *pSeq, 
		const std:: string &fileName, const std:: string &preprocessScript,
		PreprocessedFileReader *pPrepFlieScanner, std:: string *pErrorMessage);

//bool countLinesOfFile(
//		const std:: string &fileName, const std:: string &preprocessScript,
//		PreprocessedFileReader *pPreprocessedFileReader, std:: string *pErrorMessage,
//		size_t *pLoc, // loc including comments or whitespaces
//		size_t *pSloc, // count of lines that contains tokens (comments will be excluded)
//		size_t *pLocOfAvailableTokens, // count of lines that contains specified tokens by availableTokens
//		const boost::dynamic_bitset<> *pAvailableTokens);

class ProgressReporter {
private:
	std:: ostream *pOutput;
	boost::int64_t progressDone;
	boost::int64_t progressReportResolution;
	boost::int64_t startValue;
	boost::int64_t endValue;
public:
	ProgressReporter()
		: pOutput(NULL), progressDone(0), progressReportResolution(80), startValue(0), endValue(1)
	{
	}
	ProgressReporter(const ProgressReporter &right)
		: pOutput(right.pOutput), progressDone(right.progressDone), 
				progressReportResolution(right.progressReportResolution),
				startValue(right.startValue), endValue(right.endValue)
	{
	}
	void attachOutput(std:: ostream *pOutput_)
	{
		pOutput = pOutput_;
	}
	void clear()
	{
		progressDone = 0;
	}
	void setStartEnd(boost::int64_t startValue_, boost::int64_t endValue_)
	{
		assert(endValue_ >= startValue_);
		startValue = startValue_;
		endValue = endValue_;
		progressDone = 0;
	}
	void reportProgress(boost::int64_t curValue)
	{
		if (pOutput == NULL) {
			return;
		}

		// check for avoiding zero-divide error
		if (startValue >= endValue) {
			return;
		}

		boost::int64_t p = (curValue - startValue) * progressReportResolution / (endValue - startValue);
		while (progressDone < p) {
			if (progressDone % (progressReportResolution/5) == 0) {
				(*pOutput) << (100 * progressDone / progressReportResolution) << "%";
			}
			(*pOutput) << (progressDone % 2 == 0 ? "." : "\b:");
			++progressDone;
		}
	}
	void reportDone()
	{
		if (pOutput == NULL) {
			return;
		}
		
		reportProgress(endValue);
		if (progressDone % (progressReportResolution/5) == 0) {
			(*pOutput) << (100 * progressDone / progressReportResolution) << "%";
		}
		(*pOutput) << std:: endl;
	}
};

extern const std:: string CCFINDERX; //= "CCFinderX";
extern const std:: string LICENSE1; //=  "lic";
extern const std:: string LICENSE2; //=  "ens";
extern const std:: string LICENSE3; //=  "e i";
extern const std:: string LICENSE4; //=  "s e";
extern const std:: string LICENSE5; //=  "xpi";
extern const std:: string LICENSE6; //=  "red";

extern bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines);

class ThreadFunction
{
private:
	int maxWorkerThreads;
public:
	ThreadFunction();
	std::pair<int, std::string> scanOption(const std::string &arg1, const std::string &arg2);
	void applyToSystem();
	int getNumber() const;
	std::string getVerboseMessage() const;
};

#endif // CCFXCOMMON_H
