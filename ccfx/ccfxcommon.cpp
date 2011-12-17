#include <fstream>
#include <map>
#include <iostream>

#if defined USE_OPENMP

#if not defined _OPENMP
#error "the compiler doesn't support OpenMP"
#endif

#include <omp.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "../common/ffuncrenamer.h"

#include "../common/unportable.h"
#include "../common/utf8support.h"
#include "../common/filestructwrapper.h"
#include "ccfxconstants.h"
#include "ccfxcommon.h"

static Decoder defaultDecoder;

std:: string SYS2INNER(const std::string &systemString)
{
	return toUTF8String(defaultDecoder.decode(systemString));
}

std:: string INNER2SYS(const std::string &innerString)
{
	return defaultDecoder.encode(toWStringV(innerString));
}

const std::string PreprocessedFileReader::PREFIX = "prefix:";
const std::string PreprocessedFileReader::SUFFIX = "suffix:";

const char PARAMETER_SEPARATOR = '|';

const std::string PREPROCESSED_FILE_POSTFIX = "preprocessed_file_postfix";

std::string join(const std::vector<std::string> &fields, const std::string &separator)
{
	if (fields.empty()) {
		return "";
	}

	std::string buffer = fields[0];
	for (size_t i = 1; i < fields.size(); ++i) {
		buffer += separator;
		buffer += fields[i];
	}

	return buffer;
}

bool getPreprocessedSequenceOfFile(std:: vector<ccfx_token_t> *pSeq, 
		const std:: string &fileName, const std:: string &postfix,
		PreprocessedFileReader *pPreprocessedFileReader, std:: string *pErrorMessage)
{
	PreprocessedFileReader &preprocessedFileReader = *pPreprocessedFileReader;

	(*pSeq).clear();
	(*pSeq).push_back(0);

	if (! preprocessedFileReader.readFile(fileName, postfix, pSeq)) {
		*pErrorMessage = std:: string("can't open a preprocessed file of '") + fileName + "' (#5)";
		return false;
	}

	return true;
}

//bool PreprocessPackLookup::read(std::vector<unsigned char> *pBuffer, const std::string &filename, const std::string &postfix) const
//{
//	std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> > &packLookup = preprocessPackLookup[postfix];
//
//	std::string dir;
//	std::string fname;
//	std::string ext;
//
//	if (! splitpath(filename, &dir, &fname, &ext)) {
//		return false;
//	}
//
//	dir = SYS2INNER(dir);
//	fname = SYS2INNER(fname);
//	ext = SYS2INNER(ext);
//
//#if defined OS_WIN32
//	boost::algorithm::replace_all(dir, "\\", "/");
//#endif
//	if (! dir.empty() && dir[dir.length() - 1] == '/') {
//		dir.erase(dir.length() - 1);
//	}
//
//	boost::shared_ptr<ZipFileAndFilename> fai;
//	std::string faiPath;
//	std::vector<std::string> fields;
//	boost::algorithm::split(fields, dir, boost::is_any_of("/"));
//	while (true) {
//		std::string p = join(fields, "/");
//		std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> >::iterator i
//				= packLookup.find(p);
//		if (i != packLookup.end()) {
//			if (i->second.get() != NULL) {
//				fai = i->second;
//				faiPath = p;
//				break; // while
//			}
//		}
//		else {
//			std::string zipfilePath = join(fields, file_separator()) + file_separator() + "_" + postfix + ".cache";
//			ZipFileAndFilename *pZfai = NULL;
//			try {
//				{
//					zipfile::ZipFile zf;
//					zf.open(INNER2SYS(zipfilePath), "r", zipfile::ZIP_DEFLATED);
//					pZfai = new ZipFileAndFilename;
//					pZfai->zf.swap(zf);
//				}
//				pZfai->zf.namelist(&pZfai->filenames);
//				std::sort(pZfai->filenames.begin(), pZfai->filenames.end());
//				fai.reset(pZfai);
//				packLookup[p] = fai;
//				faiPath = p;
//				break; // while
//			}
//			catch (zipfile::ZipError &e) {
//				boost::shared_ptr<ZipFileAndFilename> nullPtr;
//				packLookup[p] = nullPtr;
//			}
//		}
//		if (fields.empty() 
//				|| fields.size() == 1 && fields[0] == "." 
//#if defined _OS_WIN32
//				|| fields.size() == 1 && fields[0][fields[0].length() - 1] == ':' /*like "C:" */
//#else
//				|| fields.size() == 1 && fields[0] == "") 
//#endif
//		{
//			break; // while
//		}
//		fields.pop_back();
//	}
//	
//	if (fai.get() == NULL) {
//		return false;
//	}
//
//	std::string relativeDirPath = dir.substr(faiPath.length());
//	if (! relativeDirPath.empty() && relativeDirPath[0] == '/') {
//		relativeDirPath = relativeDirPath.substr(1);
//	}
//	// for example, when dir == "a/b/c" and faiPath == "a/b", then relativeDirPath = "c"
//
//	std::string relativeFilePath;
//	if (! relativeDirPath.empty()) {
//		relativeFilePath = relativeDirPath + "/";
//	}
//	relativeFilePath += fname;
//	relativeFilePath += ext;
//	relativeFilePath += postfix;
//	std::string sRelativeFilePath = INNER2SYS(relativeFilePath);
//	if (std::binary_search(fai->filenames.begin(), fai->filenames.end(), sRelativeFilePath)) {
//		fai->zf.read(pBuffer, sRelativeFilePath);
//		return true;
//	}
//#if defined OS_WIN32
//	if (sRelativeFilePath.find('\\') != std::string::npos) {
//		if (std::binary_search(fai->filenames.begin(), fai->filenames.end(), relativeFilePath)) {
//			fai->zf.read(pBuffer, relativeFilePath);
//			return true;
//		}
//	}
//#endif
//
//	return false;
//}
//
//void PreprocessPackLookup::clear()
//{
//	std::map<std::string/* postfix */, std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> > >::iterator j;
//	for (j = preprocessPackLookup.begin(); j != preprocessPackLookup.end(); ++j) {
//		std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> > &packLookup = j->second;
//		std::map<std::string/* directory path */, boost::shared_ptr<ZipFileAndFilename> >::iterator i;
//		for (i = packLookup.begin(); i != packLookup.end(); ++i) {
//			ZipFileAndFilename *p = i->second.get();
//			if (p != NULL) {
//				p->zf.close();
//			}
//		}
//	}
//	preprocessPackLookup.clear();
//}
//
//void PreprocessPackLookup::swap(PreprocessPackLookup &rhs)
//{
//	preprocessPackLookup.swap(rhs.preprocessPackLookup);
//}

bool PreprocessedFileRawReader::readLines(const std::string &fileName0, const std::string &postfix, std::vector<std::string> *pLines) const
{
	while (prepDirsWithoutPathSeparator.size() < prepDirs.size()) {
		size_t i = prepDirsWithoutPathSeparator.size();
		const std::string &d = prepDirs[i];
		std::string utf8d = SYS2INNER(d);
		int c;
		if (utf8d.length() >= 1 && ((c = utf8d[utf8d.length() - 1]) == '/' || c == '\\')) {
			utf8d.resize(utf8d.length() - 1);
		}
		prepDirsWithoutPathSeparator.push_back(INNER2SYS(utf8d));
	}

	std::vector<std::string> &lines = *pLines;
	lines.clear();

	std::string fileName = fileName0;
	for (size_t i = 0; i < prepDirsWithoutPathSeparator.size(); ++i) {
		const std::string &prepDir = prepDirsWithoutPathSeparator[i];
		if (fileName0.length() > prepDir.length() + 1 && boost::algorithm::starts_with(fileName0, prepDir)) {
			int ch = fileName0[prepDir.length()];
			if (ch == '/' || ch == '\\') {
				fileName = prepDir;
				fileName.append(fileName0.substr(prepDir.length(), 1));
				fileName.append(".ccfxprepdir");
				fileName.append(fileName0.substr(prepDir.length()));
				break; // for i
			}
		}
	}
	
	bool fileRead = false;
	std::vector<unsigned char> buf;
	do {
		FileStructWrapper pf(fileName + postfix, "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
		if (! (bool)pf) {
			break; // do-while
		}
		FSEEK(pf, 0, SEEK_END);
		int fileSize = FTELL(pf);
		if (fileSize < 0) {
			break; // do-whille
		}
		if (fileSize == 0) {
			fileRead = true;
			break; // do-while
		}
		FSEEK(pf, 0, SEEK_SET);
		buf.resize(fileSize);
		FREAD(&buf[0], 1, fileSize, pf);
		fileRead = true;
	} while (false);

	if (! fileRead) {
		return false;
	}

	size_t bi = 0;
	while (bi < buf.size()) {
		size_t bj = bi;
		while (bj < buf.size()) {
			int ch = buf[bj];
			if (ch == '\n' || ch == '\r') {
				break;
			}
			++bj;
		}
		size_t bk = bj;
		{
			int ch;
			while (bk < buf.size() && ((ch = buf[bk]) == '\n' || ch == '\r')) {
				++bk;
			}
		}

		if (bj == bi && bk > buf.size()) {
			break; // while bi
		}
		lines.resize(lines.size() + 1);
		lines.back().assign((const char *)&buf[bi], bj - bi);
		// 2008/01/15
		//std::string line((const char *)&buf[bi], bj - bi);
		//if (line.length() == 0 && bk >= buf.size()) {
		//	break; // while bi
		//}
		//lines.resize(lines.size() + 1);
		//lines.back().swap(line);

		bi = bk;
	}

	return true; // success
}

bool PreprocessedFileReader::readFile(const std:: string &fileName, const std::string &postfix,
		std:: vector<ccfx_token_t> *pSeq)
{
	std:: vector<ccfx_token_t> &seq = *pSeq;
	special_string_map<size_t/* pos */> parameterValueTable;

	assert(! seq.empty() && seq.back() == 0); // check delimiter is found

	std::vector<std::string> lines;
	if (! rawReader.readLines(fileName, postfix, &lines)) {
		return false;
	}

	for (size_t li = 0; li < lines.size(); ++li) {
		std::string &line = lines[li];
		if (line.empty()) {
			assert(false); //debug
		}
		std::string::size_type p = line.rfind('\t');
		if (p == std::string::npos) {
			return false;
		}
		std:: string token(line.substr(p + 1));
		if (token.empty()) {
			return false; // fail
		}
		boost::optional<std::string> normalizedToken = normalizeNonParameterToken(token);
		if (normalizedToken) {
			ccfx_token_t code = allocCode(*normalizedToken);
			seq.push_back(code);
		}
		else {
			// is a parameter token
			size_t curPos = seq.size();
			special_string_map<size_t/* pos */>::iterator i = parameterValueTable.find(token);
			if (i == parameterValueTable.end()) {
				parameterValueTable[token] = curPos;
				seq.push_back(-1); // opened parameter token
			}
			else {
				ccfx_token_t code = to_displacement(curPos, i->second);
				seq.push_back(code);
				i->second = curPos;
			}
		}
	}
	
	if (seq.empty() || seq.back() != 0) {
		seq.push_back(0); // push delimiter
	}

	return true; // success
}

bool PreprocessedFileReader::readFileByName(const std:: string &fileName, std:: vector<ccfx_token_t> *pSeq)
{
	std:: vector<ccfx_token_t> &seq = *pSeq;
	special_string_map<size_t/* pos */> parameterValueTable;

	assert(! seq.empty() && seq.back() == 0); // check delimiter is found

	std::vector<std::string> lines;
	if (! get_raw_lines(fileName, &lines)) {
		return false;
	}

	for (size_t li = 0; li < lines.size(); ++li) {
		std::string &line = lines[li];
		std::string::size_type p = line.rfind('\t');
		if (p == std::string::npos) {
			return false;
		}
		std:: string token(line.substr(p + 1));
		if (token.empty()) {
			return false; // fail
		}
		boost::optional<std::string> normalizedToken = normalizeNonParameterToken(token);
		if (normalizedToken) {
			ccfx_token_t code = allocCode(*normalizedToken);
			seq.push_back(code);
		}
		else {
			size_t curPos = seq.size();
			special_string_map<size_t/* pos */>::iterator i = parameterValueTable.find(token);
			if (i == parameterValueTable.end()) {
				parameterValueTable[token] = curPos;
				seq.push_back(-1); // opened parameter token
			}
			else {
				ccfx_token_t code = to_displacement(curPos, i->second);
				seq.push_back(code);
				i->second = curPos;
			}
		}
	}
	
	if (seq.empty() || seq.back() != 0) {
		seq.push_back(0); // push delimiter
	}

	return true; // success
}

bool PreprocessedFileReader::countLinesOfFile(const std:: string &fileName, const std::string &postfix,
	size_t *pLoc, // loc including comments or whitespaces
	size_t *pSloc, // count of lines that contains tokens (comments will be excluded)
	size_t *pLocOfAvailableTokens, // count of lines that contains specified tokens by availableTokens
	const boost::dynamic_bitset<> *pAvailableTokens)
{
	const std:: string EOFToken = "eof";

	size_t loc = 0;
	size_t sloc = 0;
	size_t lastLineNumber = 0;
	size_t locOfAvailableTokens = 0;
	size_t lastLineNumberOfAvailableTokens = 0;

	std::vector<std::string> lines;
	if (! rawReader.readLines(fileName, postfix, &lines)) {
		return false;
	}

	for (size_t i = 0; i < lines.size(); ++i) {
		std::string &str = lines[i];
		if (pSloc != NULL || pLoc != NULL || pAvailableTokens != NULL) {
			std::string::size_type pos = str.find('.');
			if (pos != std:: string::npos) { 
				std:: string lineNumberStr = str.substr(0, pos);
				char *p;
				size_t lineNumber = (size_t)strtol(lineNumberStr.c_str(), &p, 16);
				size_t pos = str.rfind('\t');
				if (pos != std:: string::npos && str.substr(pos + 1) == EOFToken) {
					loc = lineNumber - 1;
				}
				else {
					if (lineNumber > lastLineNumber) {
						++sloc;
						lastLineNumber = lineNumber;
					}
					if (pAvailableTokens != NULL && i < (*pAvailableTokens).size() && (*pAvailableTokens).test(i)) {
						if (lineNumber > lastLineNumberOfAvailableTokens) {
							++locOfAvailableTokens;
							lastLineNumberOfAvailableTokens = lineNumber;
						}
					}
				}
			}
		}
	}
	
	if (pSloc != NULL) {
		*pSloc = sloc;
	}
	if (pLoc != NULL) {
		if (loc == 0) {
			*pLoc = lastLineNumber;
		}
		else {
			*pLoc = loc;
		}
	}
	if (pLocOfAvailableTokens) {
		*pLocOfAvailableTokens = locOfAvailableTokens;
	}

	return true; // success
}

//bool readLine(std::string *pLine, FILE *pFile)
//{
//	std::string &line = *pLine;
//	line.clear();
//
//	const size_t buffer_size = 1024;
//	char buffer[buffer_size];
//
//	while (fgets(buffer, buffer_size, pFile) != NULL) {
//		line.append(buffer);
//		int ch;
//		if ((! line.empty()) && ((ch = line[line.length() - 1]) == '\n' || ch == '\r')) {
//			break; // while
//		}
//	}
//	if (line.empty()) {
//		return false;
//	}
//
//	int ch;
//	while ((! line.empty()) && ((ch = line[line.length() - 1]) == '\n' || ch == '\r')) {
//		line.resize(line.length() - 1);
//	}
//
//	return true;
//}
//
//bool PackFile::setPackFile(const std::string &packFileName)
//{
//	if (pPackFile != NULL) {
//		fclose(pPackFile);
//		pPackFile = NULL;
//	}
//
//	packFilePath = packFileName;
//	pPackFile = fopen(packFilePath.c_str(), "rb");
//	if (pPackFile == NULL) {
//		return false;
//	}
//
//	std::string line;
//	while (readLine(&line, pPackFile)) {
//		if (line.empty()) {
//			break; // while
//		}
//		fileNamesInPackFile.push_back(line);
//	}
//
//	fpos_t pos;
//	fgetpos(pPackFile, &pos);
//	filePosesInPackFile.push_back(pos);
//
//	return true;
//}
//
//bool PackFile::getFileNames(std::vector<std::string> *pFileNames)
//{
//	std::vector<std::string> &fileNames = *pFileNames;
//	fileNames.clear();
//
//	if (packFilePath.empty()) {
//		return false;
//	}
//
//	if (pPackFile == NULL) {
//		bool success = setPackFile(packFilePath);
//		if (! success) {
//			return false;
//		}
//	}
//
//	fileNames.reserve(fileNamesInPackFile.size());
//
//	const std::string fileSep = file_separator();
//	std::string dir;
//	split_path(packFilePath, &dir, NULL);
//
//	if (dir.empty()) {
//		for (size_t i = 0; i < fileNamesInPackFile.size(); ++i) {
//			std::string fn = fileNamesInPackFile[i];
//			assert(fn.substr(0, fileSep.length()) == fileSep);
//			fileNames.push_back(fn.substr(fileSep.length()));
//		}
//	}
//	else {
//		for (size_t i = 0; i < fileNamesInPackFile.size(); ++i) {
//			std::string fn = fileNamesInPackFile[i];
//			fileNames.push_back(dir + fn);
//		}
//	}
//
//	return true;
//}
//
//bool PackFile::readFile(int fileIndexInPackFile, std::vector<char> *pValue)
//{
//}
//

bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines)
{
	std:: ifstream file;
	file.open(file_path.c_str(), std:: ios::in | std:: ios::binary);
	if (! file.is_open()) {
		return false;
	}

	std:: vector<std:: string> lines;
	std:: string str;
	while (! file.eof()) {
		std:: getline(file, str, '\n');
		if (str.empty() && file.eof()) {
			break; // while
		}
		if (! str.empty() && str[str.length() - 1] == '\r') {
			str.resize(str.length() - 1);
		}
		lines.push_back(str);
	}
	if (! lines.empty() && lines.back().length() == 0) {
		lines.pop_back();
	}

	(*pLines).swap(lines);
	
	file.close();

	return true;	
}

//bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines)
//{
//	std:: ifstream file;
//	std:: istream *pInput = NULL;
//
//	if (file_path == "@") {
//		pInput = &std::cin;
//	}
//	else {
//		file.open(file_path.c_str(), std:: ios::in | std:: ios::binary);
//		if (! file.is_open()) {
//			return false;
//		}
//		pInput = &file;
//	}
//
//	std:: vector<std:: string> lines;
//	std:: string str;
//	while (! (*pInput).eof()) {
//		std:: getline(*pInput, str, '\n');
//		if (str.empty() && (*pInput).eof()) {
//			break; // while
//		}
//		if (str.length() >= 1 && str[str.length() - 1] == '\r') {
//			str.resize(str.length() - 1);
//		}
//		lines.push_back(str);
//	}
//	if (! lines.empty() && lines.back().length() == 0) {
//		lines.pop_back();
//	}
//
//	(*pLines).swap(lines);
//	
//	if (pInput == &file) {
//		file.close();
//	}
//
//	return true;	
//}

namespace {

int read_script_table_i(std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > *pOutput, 
		const std::string &prepScriptDescriptionFile)
{
	assert(pOutput != NULL);
	std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > &data = *pOutput;

	std:: vector<std:: string> lines;
	if (! get_raw_lines(prepScriptDescriptionFile, &lines)) {
		std:: cerr << "error: can't read a script setting file" << std:: endl;
		return 2;
	}

	for (size_t i = 0; i < lines.size(); ++i) {
		std:: vector<std:: string> fields;
		boost::split(fields, lines[i], boost::is_any_of("="));
		if (fields.size() != 2) {
			std:: cerr << "error: invalid line in the script setting file" << std:: endl;
		}
		std::string scriptFile = fields[0];
		std::vector<std::string> exts;
		boost::split(exts, fields[1], boost::is_any_of(",:;"));
		
		for (size_t j = 0; j < exts.size(); ++j) {
            const std::string &ext = exts[j];
			if (ext.empty()) {
				continue;
			}

			if (! boost::starts_with(ext, ".")) {
				std:: cerr << "error: invalid line in the script setting file" << std:: endl;
				return 2;
			}
			if (! (scriptFile.length() > 0)) {
				std:: cerr << "error: invalid line in the script setting file" << std:: endl;
				return 2;
			}
			
			data.push_back(std::pair<std::string, std::string>(ext, scriptFile));
		}
	}

	return 0; // success
}

} // namespace

int read_script_table(std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > *pOutput, const std::string &argv0, 
		const boost::optional<std::vector<std::string> > &oOptionalPrepScriptDescriptionFiles)
{
	assert(pOutput != NULL);
	std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > &data = *pOutput;

	std::vector<std::string> descFiles;
	descFiles.push_back(make_filename_on_the_same_directory("ccfx_prep_scripts.ini", argv0));

	if (oOptionalPrepScriptDescriptionFiles) {
		const std::vector<std::string> &dfs = *oOptionalPrepScriptDescriptionFiles;
		descFiles.insert(descFiles.end(), dfs.begin(), dfs.end());
	}

	for (std::vector<std::string>::const_iterator i = descFiles.begin(); i != descFiles.end(); ++i) {
		int r = read_script_table_i(&data, *i);
		if (r != 0) return r;
	}

	return 0; // success
}

ThreadFunction::ThreadFunction()
{
	maxWorkerThreads = 0;

	boost::optional<std::string> v = getenvironmentvariable("CCFINDERX_MAX_WORKER_THREADS");
	if (v) {
		try {
			maxWorkerThreads = boost::lexical_cast<int>(*v);
		}
		catch (boost::bad_lexical_cast &) {
			// just neglect it.
		}
	}
}

std::pair<int, std::string> ThreadFunction::scanOption(const std::string &arg1, const std::string &arg2)
{
	std::string value;
	if (arg1 == "-threads") {
		if (arg2.empty()) {
			return std::pair<int, std::string>(-1, "option --threads requires an argument");
		}
		value = arg2;
	}
	else if (boost::algorithm::starts_with(arg1, "--threads=")) {
		value = arg1.substr(std::string("--threads=").length());
	}
	else {
		return std::pair<int, std::string>(0, "");
	}

	try {
		int num = boost::lexical_cast<int>(value);
#if defined USE_OPENMP
		int systemMax = omp_get_max_threads();
		if (num > 2 * systemMax) {
			return std::pair<int, std::string>(-1, "too large value for --threads");
		}
#endif
		maxWorkerThreads = num;
		return std::pair<int, std::string>(1, "");
	}
	catch (boost::bad_lexical_cast &) {
		return std::pair<int, std::string>(-1, "invalid argument is given to option --threads");
	}
}

void ThreadFunction::applyToSystem()
{
#if defined USE_OPENMP
	int systemMax = omp_get_max_threads();
	int m = maxWorkerThreads < systemMax ? maxWorkerThreads : systemMax;
	if (m == 0) {
		omp_set_num_threads(1);
	}
	else {
		omp_set_num_threads(m);
	
	}
#endif
}

int ThreadFunction::getNumber() const
{
	return maxWorkerThreads;
}

std::string ThreadFunction::getVerboseMessage() const
{
	return (boost::format("max worker threads: %d") % maxWorkerThreads).str();
}
