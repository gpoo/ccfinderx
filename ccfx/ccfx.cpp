#include <cassert>
#include <cstdlib>
#include <map>
#include <set>
#include "../common/hash_map_includer.h"
#include "../common/hash_set_includer.h"
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

#if defined USE_OPENMP
#include <omp.h>
#endif

#include <boost/format.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "../newengine/clonedetector.h"
#include "../common/utf8support.h"
#include "../common/datastructureonfile.h"
#include "../common/base64encoder.h"
#include "../common/unportable.h"
#include "../common/filestructwrapper.h"

#if defined _MSC_VER
#include <windows.h>
#include "../common/win32util.h"
#endif

#include "../common/ffuncrenamer.h"

#include "ccfxconstants.h"
#include "preprocessorinvoker.h"
#include "ccfxcommon.h"
#include "rawclonepairdata.h"
#include "shapedfragmentcalculator.h"

#include "transformermain.h"
#include "metricmain.h"
#include "prettyprintmain.h"
#include "filteringmain.h"
#include "findfilemain.h"

using namespace rawclonepair;

const std:: string LICENSE6 =  "red\0white";

//std:: string tolower(const std:: string &str)
//{
//	std:: vector<char> buf;
//	buf.resize(str.length());
//
//	bool flipAtLeastOne = false;
//	for (size_t i = 0; i < str.length(); ++i) {
//		int ch = str[i];
//		if ('A' <= ch && ch <= 'Z') {
//			ch = ch - 'A' + 'a';
//			flipAtLeastOne = true;
//		}
//		buf[i] = ch;
//	}
//
//	if (flipAtLeastOne) {
//		return std:: string(&buf[0], str.length());
//	}
//	else {
//		return str;
//	}
//}

class QueryMain {
private:
	std:: string inputFile;
	std:: string outputFile;
	bool optionVerbose;
	std:: vector<std:: vector<std:: string> > requireTokenPatterns;
	Decoder defaultDecoder;
	PreprocessedFileRawReader rawReader;


private:
	std:: string SYS2INNER(const std::string &systemString)
	{
		return toUTF8String(defaultDecoder.decode(systemString));
	}
	std:: string INNER2SYS(const std::string &innerString)
	{
		return defaultDecoder.encode(toWStringV(innerString));
	}
public:
	int main(const std::vector<std::string> &argv)
	{
		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std::cout <<
				"Usage: ccfx Q OPTIONS in.ccfxd [-o outputfile] PATTERNS..." "\n"
				"  Output clone IDs that match one of patterns." "\n"
				"Pattern" "\n"
				"  -t TOKEN[,TOKEN]: including all the tokens" "\n" 
				//"Option" "\n"
				//"  -n dir: specify directory where preprocessed files are created." "\n"
				;
			return 0;
		}

		optionVerbose = false;

		for (size_t i = 2; i < argv.size(); ++i) {
			std:: string argi = argv[i];
			if (boost::starts_with(argi, "-")) {
				if (argi == "-o") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -o requires an argument" << std:: endl;
						return 1;
					}
					outputFile = argv[i + 1];
					++i;
				}
				else if (argi == "-v") {
					optionVerbose = true;
				}
				else if (argi == "-t") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -o requires an argument" << std:: endl;
						return 1;
					}
					std:: string s = argv[i + 1];
					++i;
					std::vector<std::string> v;
					boost::split(v, s, boost::is_any_of(","));
					requireTokenPatterns.push_back(v);
				}
				//else if (argi == "-n") {
				//	if (! (i + 1 < argv.size())) {
				//		std:: cerr << "error: option -n requires an argument" << std:: endl;
				//		return 1;
				//	}
				//	std:: string s = argv[i + 1];
				//	rawReader.addPreprocessFileDirectory(s);
				//	++i;
				//}
				else {
					std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (inputFile.empty()) {
					inputFile = argi;
					force_extension(&inputFile, pairBinaryDiploidExtension);
				}
				else {
					std:: cerr << "error: too many command-line arguments" << std:: endl;
					return 1;
				}
			}
		}

		if (inputFile.empty()) {
			inputFile = "a" + pairBinaryDiploidExtension;
		}

		return do_calculation(inputFile, outputFile);
	}
private:
	static bool compileRequireTokenPatterns(std:: vector<std:: vector<ccfx_token_t> > *pCompiled, 
			const std:: vector<std:: vector<std:: string> > &patterns, const PreprocessedFileReader &scanner)
	{
		(*pCompiled).clear();

		bool allPatternCompiled = true;
		for (size_t i = 0; i < patterns.size(); ++i) {
			const std:: vector<std:: string> &pattern = patterns[i];
			std:: vector<ccfx_token_t> compd;
			size_t j;
			for (j = 0; j < pattern.size(); ++j) {
				ccfx_token_t code = scanner.getCode(pattern[j]);
				if (code == 0) {
					break; // for j
				}
				else {
					compd.push_back(code);
				}
			}
			if (j < pattern.size()) {
				allPatternCompiled = false;
			}
			else {
				(*pCompiled).push_back(compd);
			}
		}
		return allPatternCompiled;
	}

	static bool requireTokenPatternMatch(const std:: vector<ccfx_token_t> &seq, int begin, int end, 
			const std:: vector<std:: vector<ccfx_token_t> > &patterns)
	{
		assert(0 <= begin && begin <= end && end < seq.size());

		for (size_t i = 0; i < patterns.size(); ++i) {
			const std:: vector<ccfx_token_t> &pattern = patterns[i];
			std:: vector<int> itemFoundCount;
			itemFoundCount.resize(pattern.size(), 0);
			for (size_t j = begin; j < end; ++j) {
				ccfx_token_t t = seq[j];
				std:: vector<ccfx_token_t>::const_iterator k = std:: find(pattern.begin(), pattern.end(), t);
				if (k != pattern.end()) {
					++itemFoundCount[k - pattern.begin()];
				}
			}
			bool allItemFound = true;
			for (size_t j = 0; j < itemFoundCount.size(); ++j) {
				if (itemFoundCount[j] == 0) {
					allItemFound = false;
					break;
				}
			}
			if (allItemFound) {
				return true;
			}
		}
		return false;
	}

	int do_calculation(const std:: string &inputFile, const std:: string &outputFile)
	{
		if (optionVerbose) {
			std:: cerr << "> reading clone data file" << std:: endl;
		}

		RawClonePairFileAccessor acc;
		if (! acc.open(inputFile,
				rawclonepair::RawClonePairFileAccessor::CLONEDATA
				| rawclonepair::RawClonePairFileAccessor::FILEDATA)) {
			std:: cerr << "error: " << acc.getErrorMessage() << std:: endl;
			return 1;
		}

		std:: ofstream output;
		if (! outputFile.empty()) {
			output.open(outputFile.c_str(), std::ios::out);
			if (! output.is_open()) {
				std:: cerr << "error: can't create a file '" << outputFile << "'" << std:: endl;
				return 1;
			}
		}

		std:: ostream *pOutput = outputFile.empty() ? &std:: cout : &output;

		if (optionVerbose) {
			std:: cerr << "> # file: " << acc.getFileCount() << std:: endl;
			std:: cerr << "> # clone: " << acc.getCloneSetCount() << std:: endl;
		}

		std:: vector<int> fileIDs;
		acc.getFiles(&fileIDs);
		std:: sort(fileIDs.begin(), fileIDs.end());

		boost::uint64_t maxCid = acc.getMaxCloneSetID();
		std:: string tempFileRaw = ::make_temp_file_on_the_same_directory(
				theTemporaryFileBaseName ? *theTemporaryFileBaseName : outputFile, "ccfxquery", ".tmp");
		struct Match {
			bool matchAvailable;
			double match;
		public:
			Match()
				: matchAvailable(false)
			{
			}
		};
		FileStructWrapper tempFile(tempFileRaw, "w+b" F_TEMPORARY_FILE_OPTIMIZATION);
		if (! tempFile) {
			std:: cerr << "error: can not create a temporary file" << std:: endl;
			return 2;
		}
		Match match;
		for (boost::uint64_t cid = 0; cid <= maxCid; ++cid) {
			FWRITE(&match, sizeof(Match), 1, tempFile);
		}
		
		if (optionVerbose) {
			std:: cerr << "> matching" << std:: endl;
		}
		ProgressReporter rep;
		if (optionVerbose) {
			rep.attachOutput(&std:: cerr);
		}
		rep.setStartEnd(0, fileIDs.size());
		int count = 0;

		std:: vector<std:: vector<ccfx_token_t> > compiledRTPatterns;
		bool RTPatternCompleted = false;
		PreprocessedFileReader scanner;
		scanner.setRawReader(rawReader);

		std::vector<std::string> p = acc.getOptionValues(PREPROCESSED_FILE_POSTFIX);
		std::string postfix = (! p.empty()) ? p.back() : ("." + acc.getPreprocessScript() + ".ccfxprep");

		std::vector<std::string> prepDirs = acc.getOptionValues("n");
		for (std::vector<std::string>::iterator pi = prepDirs.begin(); pi != prepDirs.end(); ++pi) {
			*pi = INNER2SYS(*pi);
		}
		rawReader.setPreprocessFileDirectories(prepDirs);
		
		if (fileIDs.size() > 0) {
			size_t iPrefetched = 0;
			int fileIDPrefetched = fileIDs[iPrefetched];
			std:: vector<RawClonePair> clonePairsPrefetched;
			acc.getRawClonePairsOfFile(fileIDPrefetched, &clonePairsPrefetched);

			boost::optional<std::string> errorMessage;
			for (size_t i = 0; i < fileIDs.size(); ++i) {
				if (errorMessage) {
					break; // for i
				}
				rep.reportProgress(count);
				++count;
				int fileID = fileIDs[i];
				std:: string fileName;
				size_t length;
				acc.getFileDescription(fileID, &fileName, &length);
				fileName = INNER2SYS(fileName);

				assert(i == iPrefetched);
				std:: vector<RawClonePair> clonePairs;
				clonePairs.swap(clonePairsPrefetched);

#pragma omp parallel sections
				{
#pragma omp section
					{
#if defined USE_BOOST_POOL
						std::set<boost::uint64_t, std::less<boost::uint64_t>, boost::fast_pool_allocator<boost::uint64_t> > cidQueryNotCalculated;
#else
						std:: set<boost::uint64_t> cidQueryNotCalculated;
#endif
						{
							for (size_t j = 0; j < clonePairs.size(); ++j) {
								boost::uint64_t cid = clonePairs[j].reference;
								FSEEK64(tempFile, cid * sizeof(Match), SEEK_SET);
								Match match;
								FREAD(&match, sizeof(Match), 1, tempFile);
								if (! match.matchAvailable) {
									cidQueryNotCalculated.insert(cid);
								}
							}
						}
						
						std:: vector<ccfx_token_t> seq;
						for (size_t j = 0; j < clonePairs.size(); ++j) {
							assert(clonePairs[j].left.file == fileID);

							boost::uint64_t cid = clonePairs[j].reference;
							FSEEK64(tempFile, cid * sizeof(Match), SEEK_SET);
							FREAD(&match, sizeof(Match), 1, tempFile);
							if (! match.matchAvailable) {
								if (seq.size() == 0) {
									std:: string em;
									if (! getPreprocessedSequenceOfFile(&seq, fileName, postfix, &scanner, &em)) {
										errorMessage = em;
									}
									else {
										if (! RTPatternCompleted) {
											RTPatternCompleted = compileRequireTokenPatterns(&compiledRTPatterns, requireTokenPatterns, scanner);
										}
									}
								}

								if (requireTokenPatternMatch(seq, clonePairs[j].left.begin, clonePairs[j].left.end, compiledRTPatterns)) {
									match.matchAvailable = true;
									match.match = true;
									FSEEK64(tempFile, cid * sizeof(Match), SEEK_SET);
									FWRITE(&match, sizeof(Match), 1, tempFile);
								}
							}
						}
					}
#pragma omp section
					{
						if (++iPrefetched < fileIDs.size()) {
							fileIDPrefetched = fileIDs[iPrefetched];
							acc.getRawClonePairsOfFile(fileIDPrefetched, &clonePairsPrefetched);
						}
					}
				} // end #pragma omp sections
			}
			if (errorMessage) {
				std:: cerr << "error: " << (*errorMessage) << std:: endl;

				tempFile.remove();
				return 1;
			}
		}
		rep.reportDone();

		{
			boost::uint64_t cidCount = 0;
			boost::uint64_t cid;
			if (acc.getFirstCloneSetID(&cid)) {
				while (true) {
					FSEEK64(tempFile, cid * sizeof(Match), SEEK_SET);
					FREAD(&match, sizeof(Match), 1, tempFile);
					if (match.matchAvailable) {
						if (match.match) {
							(*pOutput) << cid << std:: endl;
						}
						++cidCount;
					}

					if (! acc.getNextCloneSetID(&cid)) {
						break;
					}
				}
			}
			if (optionVerbose) {
				std:: cerr << "> # matched clone: " << cidCount << std:: endl;
			}
		}

		tempFile.remove();
		return 0;
	}
};

const std:: string CCFINDERX = "CCFinderX";

class MySequenceHashFunction : public CloneDetector<ccfx_token_t, unsigned short>::SequenceHashFunction {
public:
	virtual unsigned short operator()(const std:: vector<ccfx_token_t> &seq, size_t begin, size_t end)
	{
		unsigned short hashValue = 0;
		int j = 0;
		for (size_t i = begin; i != end; ++i) {
			ccfx_token_t token = seq[i];
			if (token <= -1 /* opened parameter token */) {
				token = -1;
			}
			hashValue += (unsigned short)(token * token);
			//hashValue += (j + 1) * (unsigned short)(token);
			//hashValue += (1 + 5 * j) * ((unsigned short)(token));
			//hashValue *= 2; hashValue += (unsigned short)token;
			++j;
		}
		return hashValue;
	}
};

enum { DETECT_WITHIN_FILE = 1 << 0, DETECT_BETWEEN_FILES = 1 << 1, DETECT_BETWEEN_GROUPS = 1 << 2 }; 

namespace {

bool clone_matches_w_range_wfg(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	return false;
}

bool clone_matches_w_range_Wfg(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	return leftFileIndex == rightFileIndex;
}

bool clone_matches_w_range_wFg(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	if (pFileIndexToGroupIDTable == NULL) {
		// all the files are belonging to the same group
		return leftFileIndex != rightFileIndex;
	}
	else {
		const std::vector<int> &fileIndexToGroupIDTable = *pFileIndexToGroupIDTable;
		return leftFileIndex != rightFileIndex && fileIndexToGroupIDTable[leftFileIndex] == fileIndexToGroupIDTable[rightFileIndex];
	}
}

bool clone_matches_w_range_WFg(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	if (pFileIndexToGroupIDTable == NULL) {
		// all the files are belonging to the same group
		return true;
	}
	else {
		const std::vector<int> &fileIndexToGroupIDTable = *pFileIndexToGroupIDTable;
		return fileIndexToGroupIDTable[leftFileIndex] == fileIndexToGroupIDTable[rightFileIndex];
	}
}

bool clone_matches_w_range_wfG(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	if (pFileIndexToGroupIDTable == NULL) {
		// all the files are belonging to the same group
		return false;
	}
	else {
		const std::vector<int> &fileIndexToGroupIDTable = *pFileIndexToGroupIDTable;
		return fileIndexToGroupIDTable[leftFileIndex] != fileIndexToGroupIDTable[rightFileIndex];
	}
}

bool clone_matches_w_range_WfG(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	if (pFileIndexToGroupIDTable == NULL) {
		// all the files are belonging to the same group
		return leftFileIndex == rightFileIndex;
	}
	else {
		const std::vector<int> &fileIndexToGroupIDTable = *pFileIndexToGroupIDTable;
		return leftFileIndex == rightFileIndex || fileIndexToGroupIDTable[leftFileIndex] != fileIndexToGroupIDTable[rightFileIndex];
	}
}

bool clone_matches_w_range_wFG(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	return leftFileIndex != rightFileIndex;
}

bool clone_matches_w_range_WFG(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable)
{
	return true;
}

typedef bool clone_matches_w_range_func_t(int leftFileIndex, int rightFileIndex, const std::vector<int> *pFileIndexToGroupIDTable);

clone_matches_w_range_func_t *CloneMatchesWRangeTable[] = {
	&clone_matches_w_range_wfg, &clone_matches_w_range_Wfg,
	&clone_matches_w_range_wFg, &clone_matches_w_range_WFg,
	&clone_matches_w_range_wfG, &clone_matches_w_range_WfG,
	&clone_matches_w_range_wFG, &clone_matches_w_range_WFG,
};

}; // namespace (anonymous)

class CcfxClonePairListener : public CloneDetector<ccfx_token_t, unsigned short>::ClonePairListenerWithScope, private AppVersionChecker {
private:
	boost::array<boost::int32_t, 3> version;
	const std:: vector<std:: string> *pInputFiles;
	const std:: vector<size_t> *pInputFileLengths;
	const std:: vector<size_t> *pFileStartPoss;
	const std:: vector<int> *pFileIDs;
	const std::vector<int> *pFileIndexToGroupIDTable;
	std:: vector<std:: pair<ccfx_token_t, ccfx_token_t> > parens;
	shaper::ShapedFragmentsCalculator<ccfx_token_t> shapedFragmentCalculator;
	size_t targetLength;
	std:: string preprocessScript;
	std:: string outputName;
	FILE *pOutput;
	boost::uint64_t foundClones;
	std:: vector<boost::int64_t> inputFileLengthPoss;
	int shapingLevel;
	bool useParameterUnification;
	int minimumTokenSetSize;
	int detectFrom;
	clone_matches_w_range_func_t *pDetectFromFunc;
	std::pair<size_t, size_t> targetFileRange;
	std::map<int, std::vector<std::string> > inputFileRemarks;
	std::vector<std::pair<std::string/* name */, std::string/* value */> > otherOptions;
public:
	CcfxClonePairListener()
		: AppVersionChecker(APPVERSION[0], APPVERSION[1]),
		pInputFiles(NULL), pInputFileLengths(NULL), pFileStartPoss(NULL), pFileIDs(NULL), pFileIndexToGroupIDTable(NULL),
		targetLength(0), preprocessScript(),
		outputName(), pOutput(NULL), foundClones(0), inputFileLengthPoss(), 
		shapingLevel(2), useParameterUnification(true), minimumTokenSetSize(0),
		detectFrom(DETECT_WITHIN_FILE | DETECT_BETWEEN_FILES | DETECT_BETWEEN_GROUPS),
		pDetectFromFunc(CloneMatchesWRangeTable[DETECT_WITHIN_FILE | DETECT_BETWEEN_FILES | DETECT_BETWEEN_GROUPS]),
		targetFileRange(0, 0)
	{
	}
	virtual ~CcfxClonePairListener()
	{
		closeOutputFile();
	}
	void setVersion(boost::int32_t v1, boost::int32_t v2, boost::int32_t v3)
	{
		version[0] = v1;
		version[1] = v2;
		version[2] = v3;
		checkVersion(version.begin());
	}
	void swapInputFileRemarks(std::map<int, std::vector<std::string> > *pInputFileRemarks)
	{
		inputFileRemarks.swap(*pInputFileRemarks);
	}
	void addOption(const std::string &name, const std::string &value)
	{
		assert(is_name(name));
		assert(is_utf8_nocontrol(value));
		otherOptions.push_back(std::pair<std::string, std::string>(name, value));
	}
private:
	void writeVersion()
	{
		const std:: string magicNumber = "ccfxraw0";
		FWRITEBYTES(magicNumber.data(), magicNumber.length(), pOutput);
		
		for (size_t i = 0; i < version.size(); ++i) {
			boost::int32_t v = version[i];
			flip_endian(&v, sizeof(boost::int32_t));
			FWRITE(&v, sizeof(boost::int32_t), 1, pOutput);
		}
		
		const std:: string formatString = "pa:d";
		assert(formatString.length() == 4);
		FWRITEBYTES(formatString.data(), formatString.length(), pOutput);
	}
	void writeOptions_v2()
	{
		std::string str = (boost::format("b" "\t" "%d" "\n") % targetLength).str();
		FWRITEBYTES(str.data(), str.length(), pOutput);

		str = (boost::format("s" "\t" "%d" "\n") % shapingLevel).str();
		FWRITEBYTES(str.data(), str.length(), pOutput);

		str = (boost::format("u" "\t" "%s" "\n") % (useParameterUnification ? "+" : "-")).str();
		FWRITEBYTES(str.data(), str.length(), pOutput);

		str = (boost::format("t" "\t" "%d" "\n") % minimumTokenSetSize).str();
		FWRITEBYTES(str.data(), str.length(), pOutput);

		str = (boost::format("w" "\t" "f%cg%cw%c" "\n") % ((detectFrom & DETECT_BETWEEN_FILES) != 0 ? '+' : '-')
			% ((detectFrom & DETECT_BETWEEN_GROUPS) != 0 ? '+' : '-')
			% ((detectFrom & DETECT_WITHIN_FILE) != 0 ? '+' : '-')).str();
		//str = (boost::format("w" "\t" "f%cg%cw%c" "\n") % (detect_within_file ? '+' : '-')
		//	% (detect_between_files ? '+' : '-')
		//	% (detect_between_groups ? '+' : '-')).str();
		FWRITEBYTES(str.data(), str.length(), pOutput);

		for (size_t i = 0; i < otherOptions.size(); ++i) {
			const std::string &name = otherOptions[i].first;
			const std::string &value = otherOptions[i].second;
			str = (boost::format("%s\t%s\n") % name % value).str();
			FWRITEBYTES(str.data(), str.length(), pOutput);
		}

		FPUTC('\n', pOutput);
	}
	void writePreprocessorScript()
	{
		assert(preprocessScript.find('\n') == std:: string::npos);
		FWRITEBYTES(preprocessScript.data(), preprocessScript.length(), pOutput);
		FPUTC('\n', pOutput);
	}
	void writeInputFiles_v2()
	{
		Decoder defaultDecoder;

		if (pFileIDs != NULL) {
			assert((*pFileIDs).size() == (*pInputFiles).size());
		}

		const std:: vector<std:: string> &inputFiles = *pInputFiles;
		for (size_t i = 0; i < inputFiles.size(); ++i) {
			const std:: string &fn = inputFiles[i];
			std:: string ufn = toUTF8String(defaultDecoder.decode(fn));
			assert(ufn.find('\n') == std:: string::npos);
			FWRITEBYTES(ufn.data(), ufn.length(), pOutput);
			FPUTC('\n', pOutput);
			
			boost::int32_t fileID = (pFileIDs != NULL) ? (*pFileIDs)[i] : i + 1;
			flip_endian(&fileID, sizeof(boost::int32_t));
			FWRITE(&fileID, sizeof(boost::int32_t), 1, pOutput);
			
			if (pInputFileLengths != NULL) {
				boost::int32_t v = (*pInputFileLengths)[i];
				flip_endian(&v, sizeof(boost::int32_t));
				FWRITE(&v, sizeof(boost::int32_t), 1, pOutput);
			}
			else {
				// allocate space for length
				boost::int64_t pos = FTELL64(pOutput);
				inputFileLengthPoss.push_back(pos);
				boost::int32_t dummyLength;
				FWRITE(&dummyLength, sizeof(boost::int32_t), 1, pOutput);
			}
		}

		// write end mark
		{
			FPUTC('\n', pOutput);
			boost::int32_t dummyID = 0;
			boost::int32_t dummyLength = 0;
			FWRITE(&dummyID, sizeof(boost::int32_t), 1, pOutput);
			FWRITE(&dummyLength, sizeof(boost::int32_t), 1, pOutput);
		}
	}
	void writeFileRemarks()
	{
		for (std::map<int, std::vector<std::string> >::const_iterator i = inputFileRemarks.begin(); i != inputFileRemarks.end(); ++i) {
			boost::int32_t fileID = i->first;
			flip_endian(&fileID, sizeof(boost::int32_t));
			const std::vector<std::string> &texts = i->second;
			for (size_t j = 0; j < texts.size(); ++j) {
				const std::string &text = texts[j];
				assert(is_utf8_nocontrol(text));
				FWRITEBYTES(text.data(), text.length(), pOutput);
				FPUTC('\n', pOutput);
				FWRITE(&fileID, sizeof(boost::int32_t), 1, pOutput);
			}
		}
		FPUTC('\n', pOutput);
		boost::int32_t dummyID = 0;
		FWRITE(&dummyID, sizeof(boost::int32_t), 1, pOutput);
	}
	void writeDummyCloneSetRemarks()
	{
		FPUTC('\n', pOutput);
		boost::int64_t dummyID = 0;
		FWRITE(&dummyID, sizeof(boost::int64_t), 1, pOutput);
	}
public:
	void attachFileNames(const std:: vector<std:: string> *pInputFiles_)
	{
		pInputFiles = pInputFiles_;
	}
	void setMinimumLength(size_t targetLength_)
	{
		targetLength = targetLength_;
	}
	void setPreprocessScript(const std:: string &preprocessScript_)
	{
		preprocessScript = preprocessScript_;
	}
	void attachFileStartPositions(const std:: vector<size_t> *pFileStartPoss_)
	{
		pFileStartPoss = pFileStartPoss_;
	}
	void attachFileIDs(const std:: vector<int> *pFileIDs_)
	{
		pFileIDs = pFileIDs_;
	}
	void attachGroupIDs(const std::vector<int> *pFileIndexToGroupIDTable_)
	{
		pFileIndexToGroupIDTable = pFileIndexToGroupIDTable_;
	}
	void attachFileLengths(const std:: vector<size_t> *pInputFileLengths_)
	{
		pInputFileLengths = pInputFileLengths_;
		assert(pInputFiles == NULL || (*pInputFiles).size() == (*pInputFileLengths).size());
	}
	void setParens(const std:: vector<std:: pair<ccfx_token_t, ccfx_token_t> > &parens_)
	{
		parens = parens_;
		shapedFragmentCalculator.setParens(parens);
	}
	void setPrefixes(const std::vector<ccfx_token_t> &prefixes)
	{
		shapedFragmentCalculator.setPrefixes(prefixes);
	}
	void setSuffixes(const std::vector<ccfx_token_t> &suffixes)
	{
		shapedFragmentCalculator.setSuffixes(suffixes);
	}
	void setShapingLevel(int shapingLevel_)
	{
		shapingLevel = shapingLevel_;
	}
	void setUseParameterUnification(bool useParameterUnification_)
	{
		useParameterUnification = useParameterUnification_;
	}
	void setMinimumTokenSetSize(int minimumTokenSetSize_)
	{
		minimumTokenSetSize = minimumTokenSetSize_;
	}
	void setDetectFrom(int detectFrom_)
	{
		detectFrom = detectFrom_;
		pDetectFromFunc = CloneMatchesWRangeTable[detectFrom];
	}
	bool createOutputFile(const std:: string &outputName_)
	{
		if (outputName_.length() == 0) {
			return false;
		}

		if (pInputFiles == NULL) {
			assert(false);
		}

		outputName = outputName_;
		pOutput = fopen(outputName.c_str(), "wb");
		if (pOutput == NULL) {
			return false; // can not create an output file
		}

		writeVersion();
		writeOptions_v2();
		writePreprocessorScript();
		writeInputFiles_v2();
		writeFileRemarks();

		return true;
	}
	void closeOutputFile()
	{
		if (pOutput != NULL) {
			writeDummyCloneSetRemarks();
			if (! inputFileLengthPoss.empty()) {
				if (pInputFileLengths != NULL) {
					assert(inputFileLengthPoss.size() == (*pInputFileLengths).size());
					for (size_t i = 0; i < inputFileLengthPoss.size(); ++i) {
						FSEEK64(pOutput, inputFileLengthPoss[i], SEEK_SET);
						boost::int32_t len = (*pInputFileLengths)[i];
						flip_endian(&len, sizeof(boost::int32_t));
						FWRITE(&len, sizeof(boost::int32_t), 1, pOutput);
					}
				}
			}

			fclose(pOutput);
			pOutput = NULL;
		}
	}
	void discardOutputFile()
	{
		if (pOutput != NULL) {
			fclose(pOutput);
			pOutput = NULL;
		}
	}
	virtual bool codeCheck(size_t posA, size_t length)
	{
		if (shapingLevel >= 1 && ! parens.empty()) {
			assert(posA + length <= refSeq().size());
			
			shapedFragmentCalculator.setMinlengh(targetLength);
			boost::optional<shaper::ShapedFragmentPosition> pFragment = shapedFragmentCalculator.findAtLeastOne(refSeq(), posA, posA + length, shaper::HAT_FRAGMENT);
			if (! pFragment) {
				return false;
			}

			//shapedFragmentCalculator.calc(&fragments, refSeq(), posA, posA + length, shaper::HAT_FRAGMENT);
			//bool found = false;
			//for (size_t i = 0; i < fragments.size(); ++i) {
			//	const shaper::ShapedFragmentPosition &sfp = fragments[i];
			//	if (sfp.end - sfp.begin >= targetLength) {
			//		found = true;
			//		break; // for
			//	}
			//}
			//if (! found) {
			//	return false;
			//}
		}
		if (minimumTokenSetSize >= 1) {
			size_t tks = metrics::calcTKS(refSeq(), posA, posA + length);
			if (! (tks >= minimumTokenSetSize)) {
				return false;
			}
		}
		return true;
	}
	void found_scoped(size_t posA, size_t posB, size_t baseLength, boost::uint64_t cloneSetReferenceNumber)
	{
		assert(pDetectFromFunc != NULL);

		size_t length = baseLength;
		if (length < targetLength) {
			return;
		}

		++foundClones;
		
		const std:: vector<size_t> &fileStartPoss = *pFileStartPoss;
		if (pFileIDs != NULL) {
			assert((*pFileIDs).size() == fileStartPoss.size());
		}

		std:: vector<size_t>::const_iterator posAFile = std:: upper_bound(fileStartPoss.begin(), fileStartPoss.end(), posA);
		--posAFile;
		size_t posAFileIndex = posAFile - fileStartPoss.begin();

		std:: vector<size_t>::const_iterator posBFile = std:: upper_bound(fileStartPoss.begin(), fileStartPoss.end(), posB);
		--posBFile;
		size_t posBFileIndex = posBFile - fileStartPoss.begin();

		if ((*pDetectFromFunc)(posAFileIndex, posBFileIndex, pFileIndexToGroupIDTable)) {
		//if (posAFileIndex == posBFileIndex && ((detectFrom & DETECT_WITHIN_FILE) != 0)
		//		|| posAFileIndex != posBFileIndex && ((detectFrom & DETECT_BETWEEN_FILES) != 0)) {
			size_t posAFileID = fileIndexToFileID(posAFileIndex);
			size_t posBFileID = fileIndexToFileID(posBFileIndex);
			RawClonePair pd[2] = {
				RawClonePair(posAFileID, posA - *posAFile, posA - *posAFile + length, 
					posBFileID, posB - *posBFile, posB - *posBFile + length, 
					cloneSetReferenceNumber)
			};
			pd[1] = pd[0];
			pd[1].left.swap(pd[1].right);
			fwrite_RawClonePair(pd, 2, pOutput);
		}
	}
	void writeEndOfCloneDataMark()
	{
		static const RawClonePair terminator(0, 0, 0, 0, 0, 0, 0);
		fwrite_RawClonePair(&terminator, 1, pOutput);
	}
	boost::uint64_t countClones() const
	{
		return foundClones;
	}
private:
	inline size_t fileIndexToFileID(size_t index) const
	{
		if (pFileIDs == NULL) {
			assert(index < (*pInputFiles).size());
			return index + 1;
		}
		else {
			assert(index < (*pFileIDs).size());
			return (*pFileIDs)[index];
		}
	}
};

const std:: string LICENSE1 =  "lica";

std:: pair<int/* progress */, int/* total */> calc_progress(int s, int fbegin, int fend, int g)
{
	//std:: cerr << s << "," << fbegin << "," << fend << "," << g << std::endl;
	int total = s;
	int progress = fend + (fend - fbegin) * (g - fend) / (s - fend);
	return std:: pair<int, int>(progress, total);
}

const std:: string prepExtension = ".ccfxprep";
const std:: string pairBinaryDiploidExtension = ".ccfxd";

namespace {

template<typename StringFwdIterator>
void force_extension_i(std:: string *pFileName, StringFwdIterator begin, StringFwdIterator end)
{
	std:: string &t = *pFileName;

	for (StringFwdIterator i = begin; i != end; ++i) {
		const std:: string &extension = *i;
		if (boost::ends_with(t, extension)) {
			return;
		}
	}

	// here, *pFileName does not match any extension
	const std:: string &extension = *begin;
	t += extension;
}

}; // namespace

void force_extension(std:: string *pFileName, const std:: string &extension)
{
	const std:: string extensions[] = { extension };
	force_extension_i(pFileName, &(extensions[0]), &(extensions[0]) + 1);
}

void force_extension(std:: string *pFileName, const std:: string &extension1, const std:: string &extension2)
{
	const std:: string extensions[] = { extension1, extension2 };
	force_extension_i(pFileName, &(extensions[0]), &(extensions[0]) + 2);
}

void force_extension(std:: string *pFileName, const std:: string &extension1, const std:: string &extension2, const std:: string &extension3)
{
	const std:: string extensions[] = { extension1, extension2, extension3 };
	force_extension_i(pFileName, &(extensions[0]), &(extensions[0]) + 3);
}

void force_extension(std:: string *pFileName, const std:: string *extensionBegin, const std:: string *extensionEnd)
{
	force_extension_i(pFileName, extensionBegin, extensionEnd);
}

bool is_path_utf8_safe(const std::string &path)
{
	const std::string encPath = SYS2INNER(path);
	const std::string decPath = INNER2SYS(encPath);
	return decPath == path;
}

std::string normalize_path_separator(const std::string &path)
{
	std::string encPath = SYS2INNER(path);
	if (! encPath.empty()) {
		if (boost::algorithm::ends_with(encPath, file_separator())) {
			if (encPath.length() > 3) {
				encPath = encPath.substr(0, encPath.length() - 1);
			}
		}
	}
	return INNER2SYS(encPath);
}

namespace {

	class SystemError : public std::logic_error {
public:
	int errorCode;
public:
	SystemError(const std::string &errorMessage, int errorCode_)
		: std::logic_error(errorMessage), errorCode(errorCode_)
	{
	}
	SystemError(const boost::format &errorMessageF, int errorCode_)
		: std::logic_error(errorMessageF.str()), errorCode(errorCode_)
	{
	}
};

} // namespace

class CloneDetectionMain
{
private:
	preprocessor_invoker prepInvoker;
	boost::optional<std::string> preprocessScriptName;
	std::vector<InputFileData> inputFiles;
	std::map<int/* file ID */, std::vector<std::string> > inputFileRemarks;
	std:: string outputFileName;
	bool optionVerbose;
	bool optionDebugUnsort;
	bool optionDebugSortOnly;
	bool optionOnlyPreprocess;
	int optionShapingLevel;
	bool optionMajoritarianShaper;
	int optionB; // minimumCloneLength
	int optionT; // minimumTokenSetSize
	bool optionParameterUnification;
	size_t chunkSize;
	size_t bottomUnitLength;
	size_t multiply;
	int optionDetectFrom;
	std::set<std::string> maskRemarks;
	ThreadFunction threadFunction; 
	PreprocessedFileRawReader rawReader;
	bool optionParameterization;
	boost::optional<std::string> optionParseErrors;
	boost::optional<size_t> lengthLimit;
public: 
	CloneDetectionMain() : optionVerbose(false), 
			optionDebugUnsort(false), 
			optionDebugSortOnly(false),
			optionOnlyPreprocess(false),
			optionShapingLevel(2),
			optionMajoritarianShaper(true),
			optionB(50),
			optionT(12),
			optionParameterUnification(true),
			chunkSize(60 * 1024 * 1024), 
			bottomUnitLength(0), 
			multiply(0),
			optionDetectFrom(DETECT_WITHIN_FILE | DETECT_BETWEEN_FILES | DETECT_BETWEEN_GROUPS),
			optionParameterization(true),
			optionParseErrors()
	{
	}
private:
	static void modify_relative_path(std:: string *pPath, const std:: string &origin)
	{
		bool isRelative = false;
		size_t relPathStartPos = 0;
		std:: string &path = *pPath;
		const std:: string relMark1 = "./";
		const std:: string relMark2 = ".\\";
		if (boost::starts_with(path, relMark1)) {
			isRelative = true;
			relPathStartPos = relMark1.length();
		}
		else if (boost::starts_with(path, relMark2)) {
			isRelative = true;
			relPathStartPos = relMark2.length();
		}
		if (isRelative) {
			std:: string dir;
			splitpath(origin, &dir, NULL, NULL);
			if (dir.length() > 0) {
				path = dir + path.substr(relPathStartPos);
			}
		}
	}

	bool analyzeLongOptions(const std::string &argi)
	{
		if (boost::algorithm::starts_with(argi, "--errorfiles=")) {
			std::cerr << "warning: option --errorfiles is *experimental*. Be careful!" << std::endl;
			std::string fileName = argi.substr(std::string("--errorfiles=").length());
			optionParseErrors = fileName;
		}
		else if (boost::algorithm::starts_with(argi, "--prescreening=")) {
			const std::string LEN_GT_ = "LEN.gt.";
			std::string s = argi.substr(argi.find('=') + 1);
			if (! boost::algorithm::starts_with(s, LEN_GT_)) {
				throw SystemError(boost::format("error: invalid argument of option %s: '%s'") % "--prescreening" % s, 1);
			}
			std::string v = s.substr(LEN_GT_.length());
			try {
				lengthLimit = boost::lexical_cast<size_t>(v);
			}
			catch (boost::bad_lexical_cast &) {
				throw SystemError("error: invalid left value of option --prescreening=LEN.gt.", 1);
			}
		}
		else {
			return false;
		}
		return true;
	}

	int analyzeCommandLine(const std::vector<std::string> &argv)
	{
		bool optionWSpecified = false;
		bool optionOSpecified = false;

		int curGroupID = 1;

		int largestFileID = 0;

		if (theTemporaryFileBaseName) {
			prepInvoker.setTemporaryDir(*theTemporaryFileBaseName);
		}

		prepInvoker.setEncodng("char");
		std::vector<std::string> preprocessOptions;

		try {
			size_t i = 2;

			std:: string argi = argv[i];
			if (! (i < argv.size())) throw SystemError("error: preprocess script is not specified", 1);
			if (argi != "-") {
				if (! prepInvoker.setPreprocessorName(argi)) throw SystemError("error: named preprocess script not found", 1);
				preprocessScriptName = argi;
			}
			else {
				preprocessScriptName = boost::optional<std::string>();
			}
			++i;

			for (; i < argv.size(); ++i) {
				argi = argv[i];
				if (argi == "-b") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -b requires an argument", 1);
					try {
						optionB = boost::lexical_cast<int>(argv[i + 1]);
					}
					catch(boost::bad_lexical_cast &) {
						throw SystemError("error: invalid argument is given to option -b", 1);
					}
					++i;
				}
				else if (argi == "-d" "d" "debug") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -d requires an argument", 1);
					std:: string argv1 = argv[i + 1];
					if (argv1 == "u") {
						optionDebugUnsort = true;
					}
					else if (argv1 == "s") {
						optionDebugSortOnly = true;
					}
					else {
						throw SystemError("error: invalid commands for option -d", 1);
					}
					++i;
				}
				else if (argi == "-c" || argi == "-ci") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -c requires an argument", 1);
					std:: string encstr = argv[i + 1];
					prepInvoker.setEncodng(encstr);
					++i;
				}
				else if (argi == "-i") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -i requires an argument", 1);
					std:: string listFileName = argv[i + 1];
					readFileList_ac(listFileName, &largestFileID, &curGroupID);
					++i;
				}
				else if (argi == "-is") {
					assert(curGroupID < std::numeric_limits<int>::max());
					++curGroupID;
				}
				else if (argi == "-k") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -k requires an argument", 1);
					std:: string sizeStr = argv[i + 1];
					parseChunkSize_ac(sizeStr);
					++i;
				}
				else if (argi == "-k-") {
					chunkSize = 0;
				}
				else if (argi == "-o") {
					if (optionOnlyPreprocess) throw SystemError("error: option -o and -p are exclusive", 1);
					if (! (i + 1 < argv.size())) throw SystemError("error: option -o requires an argument", 1);
					//std:: string dir;
					//splitpath(argv[i + 1], &dir, NULL, NULL);
					//if (! dir.empty()) {
					//	std:: cerr << "error: argument of option -o should be a file name on the current directory" << std:: endl;
					//	return 1;
					//}
					outputFileName = argv[i + 1];
					++i;
				}
				else if (argi == "-p") {
					if (! outputFileName.empty()) throw SystemError("error: option -o and -p are exclusive", 1);
					optionOnlyPreprocess = true;
				}
				else if (argi == "-v") {
					optionVerbose = true;
				}
				else if (argi == "-s-") {
					optionShapingLevel = 0;
				}
				else if (argi == "-s") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -s requires an argument", 1);
					std:: string a1 = argv[i + 1];
					if (a1.empty()) throw SystemError("error: invalid argument is given to option -s", 1);
					if (a1 == "-") {
						optionShapingLevel = 0;
					}
					else {
						try {
							optionShapingLevel = boost::lexical_cast<int>(a1);
						}
						catch (boost::bad_lexical_cast &) {
							throw SystemError("error: invalid argument is given to option -s", 1);
						}
					}
					++i;
				}
				else if (argi == "-j") {
					optionMajoritarianShaper = true;
				}
				else if (argi == "-j-") {
					optionMajoritarianShaper = false;
				}
				else if (argi == "-u-") {
					optionParameterUnification = false;
				}
				else if (argi == "-u") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -u requires an argument", 1);
					std:: string a1 = argv[i + 1];
					if (a1 == "-") {
						optionParameterUnification = false;
					}
					else if (a1 == "+") {
						optionParameterUnification = true;
					}
					else {
						throw SystemError("error: invalid argument is given to option -u", 1);
					}
					++i;
				}
				else if (argi == "-t") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -t requires an argument", 1);
					try {
						optionT = boost::lexical_cast<int>(argv[i + 1]);
					}
					catch (boost::bad_lexical_cast &) {
						throw SystemError("error: invalid argument is given to option -t", 1);
					}
					++i;
				}
				else if (argi == "-w") {
					if (optionWSpecified == true) throw SystemError("error: option -w specified twice", 1);
					optionWSpecified = true;
					if (! (i + 1 < argv.size())) throw SystemError("error: option -w requires an argument", 1);
					std:: string a = argv[i + 1];
					boost::array<std:: pair<char, bool>, 3> params = {{ 
						std:: pair<char, bool>('w', (optionDetectFrom & DETECT_WITHIN_FILE) != 0), 
						std:: pair<char, bool>('f', (optionDetectFrom & DETECT_BETWEEN_FILES) != 0),
						std:: pair<char, bool>('g', (optionDetectFrom & DETECT_BETWEEN_GROUPS) != 0)
					}};
					if (! scanPlusMinus(&params[0], params.size(), a)) throw SystemError("error: invalid parameter for option -w", 1);
					optionDetectFrom = (params[0].second ? DETECT_WITHIN_FILE : 0) 
							| (params[1].second ? DETECT_BETWEEN_FILES : 0)
							| (params[2].second ? DETECT_BETWEEN_GROUPS : 0);
					++i;
				}
				else if (argi == "-mr") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -mr requires an argument", 1);
					std::string name = argv[i + 1];
					if (! is_name(name)) throw SystemError("error: invalid name to option -mr", 1);
					maskRemarks.insert(name);
					++i;
				}
				else if (argi == "-r") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -r requires an argument", 1);
					std::string s = argv[i + 1];
					if (! is_ascii_nocontrol(s)) throw SystemError("error: invalid string for option -r", 1);
					preprocessOptions.push_back(s);
					++i;
				}
				else if (argi == "-n-") {
					std::cerr << "warning: option -n- is deprecated. neglect it" << std::endl;
				}
				else if (argi == "-n") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -n requires an argument", 1);
					std::string dirPath = argv[i + 1];
					if (! is_path_utf8_safe(dirPath)) throw SystemError("error: invalid path for option -n", 1);
					dirPath = normalize_path_separator(dirPath);
					rawReader.addPreprocessFileDirectory(dirPath);
					++i;
				}
				else if (argi == "-d" || argi == "-dn") {
					if (! (i + 1 < argv.size())) throw SystemError("error: option -d requires an argument", 1);
					if (! preprocessScriptName) throw SystemError("error: option -d requires valid preprocess script name", 1);
					std::string dirPath = argv[i + 1];
					addSourceDirectory_ac(dirPath, &largestFileID, &curGroupID, argi == "-dn");
					++i;
				}
				else if (argi == "-pp-") {
					optionParameterization = false;
					std::cerr << "warning: option -pp- is *experimental*. Be careful!" << std::endl;
				}
				else if (this->analyzeLongOptions(argi)) {
					NULL;
				}
				else if (boost::starts_with(argi, "-")) {
					std::pair<int, std::string> r = threadFunction.scanOption(argi, (i + 1 < argv.size()) ? argv[i + 1] : "");
					if (r.first > 0) {
						i += r.first - 1;
					}
					else if (r.first < 0) {
						std::cerr << "error: " << r.second << std::endl;
						return 1;
					}
					else {
						throw SystemError(boost::format("error: unknown option: '%s'") % argi, 1);
					}
				}
				else {
					int lastFileID = inputFiles.empty() ? 0 : inputFiles.back().fileID;
					int fileID = lastFileID + 1;
					inputFiles.push_back(InputFileData(fileID, curGroupID, argi));						
					if (fileID > largestFileID) {
						largestFileID = fileID;
					}
				}
			}
		}
		catch (SystemError &e) {
			std::cerr << e.what() << std::endl;
			return e.errorCode;
		}
		if (inputFiles.size() == 0) {
			std:: cerr << "error: no input files are given" << std:: endl;
			return 1;
		}
		{
			HASH_SET<int> fileIDs;
			for (size_t i = 0; i < inputFiles.size(); ++i) {
				int fileID = inputFiles[i].fileID;
				if (fileIDs.find(fileID) != fileIDs.end()) {
					std:: cerr << "error: file ID conflict" << std:: endl;
					return 1;
				}
				fileIDs.insert(fileID);
			}
		}

		bottomUnitLength = 25;
		multiply = 1;
		if (optionB < bottomUnitLength) {
			bottomUnitLength = optionB;
		}
		else {
			multiply = optionB / bottomUnitLength;
			assert(multiply >= 1);
			assert(multiply * bottomUnitLength <= optionB);
		}

		if (! (0 <= optionShapingLevel && optionShapingLevel <= 3)) {
			std:: cerr << "error: range error for option -s" << std:: endl;
			return 1;
		}

		prepInvoker.setOptionVerbose(optionVerbose);
		boost::optional<std::string> errorMessage = prepInvoker.setPreprocessorOptions(preprocessOptions);
		if (errorMessage) {
			std::string s = *errorMessage;
			if (! s.empty()) {
				std::cerr << s << std::endl;
			}
			return 1;
		}
		prepInvoker.setMaxWorkerThreads(threadFunction.getNumber());
		prepInvoker.setPreprocessFileDirectories(rawReader.getPreprocessFileDirectories());

		if (optionVerbose) {
			std::cerr << "> " << threadFunction.getVerboseMessage() << std::endl;
		}
		threadFunction.applyToSystem();

		return 0;
	}
	void parseChunkSize_ac(const std::string &sizeStr0)
	{
		std::string sizeStr = sizeStr0;
		if (sizeStr == "-") {
			chunkSize = 0;
		}
		else if (sizeStr.length() >= 1) {
			int lastCh = sizeStr[sizeStr.length() - 1];
			if (lastCh == 'b' || lastCh == 'B') {
				sizeStr.erase(sizeStr.length() - 1);
				if (sizeStr.length() >= 1) {
					lastCh = sizeStr[sizeStr.length() - 1];
				}
				else {
					throw SystemError("error: invalid -k argument", 1);
				}
			}
			size_t factor = 1;
			switch (lastCh) {
			case 'k': case 'K':
				factor = 1024;
				sizeStr.erase(sizeStr.length() - 1);
				break;
			case 'm': case 'M':
				factor = 1024 * 1024;
				sizeStr.erase(sizeStr.length() - 1);
				break;
			case 'g': case 'G':
				throw SystemError("error: option -k can not handle 'g'", 1);
				break;
			default:
				if (! ('0' <= lastCh && lastCh <= '9')) throw SystemError("error: invalid -k argument", 1);
			}
			try {
				boost::int32_t siz = boost::lexical_cast<boost::int32_t>(sizeStr);
				chunkSize = ((size_t)siz) * factor;
			}
			catch(boost::bad_lexical_cast &) {
				throw SystemError("error: invalid -k argument", 1);
			}
		}
	}
	void addSourceDirectory_ac(const std::string &dirPath0, int *pLargestFileID, int *pCurGroupID, bool isAlsoPreprocessedDirectory)
	{
		std::string dirPath = dirPath0;
		int &largestFileID = *pLargestFileID;
		int &curGroupID = *pCurGroupID;

		if (! is_path_utf8_safe(dirPath)) throw SystemError("error: invalid path for option -d", 1);
		dirPath = normalize_path_separator(dirPath);
		if (isAlsoPreprocessedDirectory) {
			rawReader.addPreprocessFileDirectory(dirPath);
		}
		boost::optional<std::set<std::string> > extensions = prepInvoker.getExtensionsAssociatedToPreprocessor(*preprocessScriptName);
		if (! extensions) throw SystemError(boost::format("error: unknown preprocess script: '%s'") % *preprocessScriptName, 1);
		std:: vector<std:: string> files;
		std:: vector<std:: string> fs;
		if (! find_files(&fs, *extensions, dirPath)) throw SystemError(boost::format("error: can't change directory to: '%s'") % dirPath, 1);
		for (size_t j = 0; j < fs.size(); ++j) {
			assert(largestFileID < std::numeric_limits<int>::max());
			int fileID = largestFileID + 1;
			std::string filePath = fs[j];
			inputFiles.push_back(InputFileData(fileID, curGroupID, filePath));
			largestFileID = fileID;
		}
	}
	void readFileList_ac(const std::string &listFileName, int *pLargestFileID, int *pCurGroupID) 
	{
		int &largestFileID = *pLargestFileID;
		int &curGroupID = *pCurGroupID;

		std:: vector<std:: string> ifs;
		if (! get_raw_lines(listFileName, &ifs)) throw SystemError(boost::format("error: can't open a file: '%s'") % listFileName, 1);
		int fileID;
		boost::optional<int> lastFileID;
		std:: string filePath;
		for (size_t i = 0; i < ifs.size(); ++i) {
			const std:: string &s = ifs[i];
			if (boost::algorithm::starts_with(s, "-is")) {
				if (s.length() >= 3) {
					std::string extra = s.substr(3);
					while (extra.length() > 0 && (extra[0] == ' ' || extra[0] == '\t')) {
						extra = extra.substr(1);
					}
					if (! extra.empty()) throw SystemError(boost::format("error: unknown option: '%s'") % s, 1);
				}
				assert(curGroupID < std::numeric_limits<int>::max());
				++curGroupID;
			}
			else if (boost::algorithm::starts_with(s, "-n ") || boost::algorithm::starts_with(s, "-n\t")) {
				std::string dirPath = s.substr(3);
				while (dirPath.length() > 0 && (dirPath[0] == ' ' || dirPath[0] == '\t')) {
					dirPath = dirPath.substr(1);
				}
				if (! is_path_utf8_safe(dirPath)) throw SystemError("error: invalid path for option -n", 1);
				dirPath = normalize_path_separator(dirPath);
				rawReader.addPreprocessFileDirectory(dirPath);
			}
			else {
				std::vector<std::string> fields;
				boost::split(fields, s, boost::is_any_of("\t"));
				if (fields.size() >= 2 && fields[0] == ":") {
					if (! lastFileID) throw SystemError(boost::format("error: no file ID is given to a remark in file list: '%s'") % listFileName, 1);
					std::string remStr = s.substr(2);
					std::string remStrUtf8 = SYS2INNER(remStr);
					if (! is_utf8_nocontrol(remStrUtf8)) throw SystemError(boost::format("error: a control character appears in file remark in file list: '%s'") % listFileName, 1);
					inputFileRemarks[*lastFileID].push_back(remStrUtf8);
				}
				else {						
					if (fields.size() == 1) {
						assert(largestFileID < std::numeric_limits<int>::max());
						fileID = largestFileID + 1;
						filePath = s;
					}
					else if (fields.size() >= 2) {
						std:: string &numstr = fields[0];
						std:: string pathstr = fields[1];
						try {
							boost::int32_t value = boost::lexical_cast<boost::int32_t>(numstr);
							fileID = value;
							filePath = fields[1];
						}
						catch (boost::bad_lexical_cast &) {
							throw SystemError(boost::format("error: wrong file ID string in file list: '%s'") % listFileName, 1);
						}
						if (fileID <= 0) throw SystemError(boost::format("error: wrong file ID string in file list: '%s'") % listFileName, 1);
					}
					else {
						throw SystemError(boost::format("error: invalid line in file list: '%s'") % listFileName, 1);
					}
					if (fileID > largestFileID) {
						largestFileID = fileID;
					}
					lastFileID = fileID;
					modify_relative_path(&filePath, listFileName);

					inputFiles.push_back(InputFileData(fileID, curGroupID, filePath));
				}
			}
		}
	}

	static bool scanPlusMinus(std:: pair<char, bool> *aParams, size_t countOfParams, const std:: string &str)
	{
		size_t i = 0;
		while (i < str.length()) {
			char ch = str[i];
			bool found = false;
			for (size_t j = 0; ! found && j < countOfParams; ++j) {
				std:: pair<char, bool> &param = *(aParams + j);
				if (ch == param.first) {
					if (i + 1 < str.length() && (str[i + 1] == '-' || str[i + 1] == '+')) {
						param.second = str[i + 1] == '+';
						i += 2;
						found = true;
					}
					else {
						param.second = true;
						i += 1;
						found = true;
					}
				}
			}
			if (! found) {
				return false; // invalid str
			}
		}
		return true;
	}

	int readPreprocessedFile(PreprocessedFileReader *pPreprocessedFileReader, const std::string &fileName, std:: vector<ccfx_token_t> *pSeq) const
	{
		if (! preprocessScriptName) {
			if (! (*pPreprocessedFileReader).readFileByName(fileName, pSeq)) {
				std:: cerr << "error: can't find file: '" << fileName << "'" << std:: endl;
				return 2;
			}
		}

		if ((*pPreprocessedFileReader).readFile(fileName, prepInvoker.getPostfix(), pSeq)) {
			return 0;
		}
		std:: cerr << "error: can't open a preprocessed file of souce file: '" << fileName << "' (#4)" << std:: endl;
		return 2;
	}
	int fetchPreprocessedFiles(std::vector<ccfx_token_t> *pSeq, std::vector<size_t> *pFileLengths, 
			int fiStart, size_t countMaxFetched, 
			const std::vector<size_t> &selectedToInputTable, PreprocessedFileReader *pPreprocessedFileReader, 
			bool optionParameterUnification, size_t chunkSize) const
	{
		std::vector<ccfx_token_t> &seq = *pSeq;
		std::vector<size_t> &fileLengths = *pFileLengths;
		seq.clear();
		seq.push_back(0);
		fileLengths.clear();
		size_t count = 0;
		for (int fi = fiStart; (countMaxFetched == 0 || count < countMaxFetched) && fi < selectedToInputTable.size() && (chunkSize == 0 || count <= 2 || seq.size() - 1 < chunkSize); ++fi) {
			size_t prevSize = seq.size();
			const InputFileData &fileFi = inputFiles[selectedToInputTable[fi]];
			int r = readPreprocessedFile(pPreprocessedFileReader, fileFi.path, &seq);
			if (r != 0) {
				return r;
			}
			++count;
			fileLengths.push_back(seq.size() - prevSize);
		}
		if (! optionParameterUnification) {
			remove_displacement(seq.begin(), seq.end());
		}
		return 0;
	}

	void setOptionsToClonePairListener(CcfxClonePairListener *pLis) const
	{
		CcfxClonePairListener &lis = *pLis;

		lis.setVersion(APPVERSION[0], APPVERSION[1], APPVERSION[2]);
		lis.setMinimumLength(optionB);
		lis.setShapingLevel(optionShapingLevel);
		lis.addOption("j", optionMajoritarianShaper ? "+" : "-"); 
		lis.setUseParameterUnification(optionParameterUnification);
		lis.setMinimumTokenSetSize(optionT);
		if (chunkSize % (1024 * 1024) == 0) {
			lis.addOption("k", (boost::format("%dm") % (chunkSize / (1024 * 1024))).str());
		}
		else if (chunkSize % 1024 == 0) {
			lis.addOption("k", (boost::format("%dk") % (chunkSize / 1024)).str());
		}
		else {
			lis.addOption("k", (boost::format("%d") % chunkSize).str());
		}
		if (lengthLimit) {
			lis.addOption("prescreening", (boost::format("LEN.gt.%d") % *lengthLimit).str());
		}
		{
			HASH_MAP<int/* group ID */, std::vector<int/* file ID */> > groupMembers;
			int maxGroupID = 1; // 1 is the minimum group ID.
			for (size_t fi = 0; fi < inputFiles.size(); ++fi) {
				const InputFileData &ifd = inputFiles[fi];
				groupMembers[ifd.groupID].push_back(ifd.fileID);
				if (ifd.groupID > maxGroupID) {
					maxGroupID = ifd.groupID;
				}
			}
			if (groupMembers.size() <= 1) {
				// single group.
			}
			else {
				for (int groupID = 1; groupID <= maxGroupID; ++groupID) {
					HASH_MAP<int/* group ID */, std::vector<int/* file ID */> >::const_iterator i = groupMembers.find(groupID);
					if (i != groupMembers.end() && ! i->second.empty()) {
						const std::vector<int/* file ID */> &fileIDsOfTheGroup = i->second;

						std::string buf;
						std::pair<int, int> continuousIDs(-1, -1);
						for (size_t fi = 0; fi < fileIDsOfTheGroup.size(); ++fi) {
							int curFileID = fileIDsOfTheGroup[fi];
							if (continuousIDs.first < 0) {
								continuousIDs.first = curFileID;
								continuousIDs.second = curFileID;
							}
							else if (continuousIDs.second == curFileID - 1) { // if continuous
								continuousIDs.second = curFileID;
							}
							else { // if not
								if (continuousIDs.first == continuousIDs.second) {
									buf += (boost::format(",%d") % continuousIDs.first).str();
								}
								else {
									buf += (boost::format(",%d-%d") % continuousIDs.first % continuousIDs.second).str();
								}
							}
						}
						if (continuousIDs.first == continuousIDs.second) {
							buf += (boost::format(",%d") % continuousIDs.first).str();
						}
						else {
							buf += (boost::format(",%d-%d") % continuousIDs.first % continuousIDs.second).str();
						}

						assert(buf.size() >= 1);
						lis.addOption("ig", buf.substr(1)); // remove the first comma.
					}
				}
			}
		}

		if (! preprocessScriptName) {
			lis.addOption(PREPROCESSED_FILE_POSTFIX, "-");
		}
		else {
			lis.addOption(PREPROCESSED_FILE_POSTFIX, prepInvoker.getPostfix());
		}

		lis.addOption("pp", optionParameterization ? "+" : "-");
	}

	int detectClones(const std:: string &tempOutputName)
	{
		std::vector<int> emptyMasks;
		return detectClonesWithMask(tempOutputName, emptyMasks);
	}
	int detectClonesWithMask(const std:: string &tempOutputName, const std::vector<int> &maskedFiles)
	{
		CloneDetector<ccfx_token_t, unsigned short> cd;
		cd.setThreads(threadFunction.getNumber());
		MySequenceHashFunction hashFunc;

		assert(! inputFiles.empty());

		cd.setBottomUnitLength(bottomUnitLength);
		cd.setMultiply(multiply);
		size_t unitLength = cd.getUnitLength();
		assert(unitLength <= optionB);
		
		std::vector<size_t> selectedToInputTable;
		selectedToInputTable.reserve(inputFiles.size() - maskedFiles.size());
		if (! maskedFiles.empty()) {
			std::vector<int> sortedMaskedFiles = maskedFiles;
			std::sort(sortedMaskedFiles.begin(), sortedMaskedFiles.end());
			for (size_t i = 0; i < inputFiles.size(); ++i) {
				int fileID = inputFiles[i].fileID;
				std::vector<int>::const_iterator p = std::lower_bound(sortedMaskedFiles.begin(), sortedMaskedFiles.end(), fileID);
				bool found = p != sortedMaskedFiles.end() && *p == fileID;
				if (! found) {
					selectedToInputTable.push_back(i);
				}
			}
		}
		else {
			for (size_t i = 0; i < inputFiles.size(); ++i) {
				selectedToInputTable.push_back(i);
			}
		}

		ProgressReporter progressRep;
		std:: pair<int, int> t = calc_progress(selectedToInputTable.size(), 0, 0, 0);
		progressRep.setStartEnd(0, t.second);
		if (optionVerbose) {
			progressRep.attachOutput(&std:: cerr);
		}

		std:: vector<ccfx_token_t> seq;
		std:: vector<size_t> fileStartPoss;

		std::vector<int> fileIDs;
		std::vector<int> groupIDs;
		std:: vector<std:: string> fileList;
		fileList.resize(inputFiles.size());
		fileIDs.resize(inputFiles.size());
		groupIDs.resize(inputFiles.size());

		for (size_t i = 0; i < inputFiles.size(); ++i) {
			const InputFileData &ifd = inputFiles[i];
			fileList[i] = ifd.path;
			fileIDs[i] = ifd.fileID;
			groupIDs[i] = ifd.groupID;
		}

		CcfxClonePairListener lis;
		setOptionsToClonePairListener(&lis);

		if (! maskRemarks.empty()) {
			for (std::set<std::string>::const_iterator i = maskRemarks.begin(); i != maskRemarks.end(); ++i) {
				lis.addOption("mr", *i);
			}
		}
		{
			std::vector<std::string> prepDirs = rawReader.getPreprocessFileDirectories();
			for (size_t pi = 0; pi < prepDirs.size(); ++pi) {
				lis.addOption("n", SYS2INNER(prepDirs[pi]));
			}
		}
		lis.setDetectFrom(optionDetectFrom);
		lis.swapInputFileRemarks(&inputFileRemarks);
		if (! preprocessScriptName) {
			lis.setPreprocessScript("-");
		}
		else {
			std::string name = prepInvoker.getPreprocessScriptName();
			assert(name == *preprocessScriptName);
			if (name.empty()) {
				assert(false);
				return 1;
			}
			lis.setPreprocessScript(name);
		}
		lis.attachFileStartPositions(&fileStartPoss);
		lis.attachFileNames(&fileList);
		lis.attachFileIDs(&fileIDs);
		lis.attachGroupIDs(&groupIDs);

		if (! lis.createOutputFile(tempOutputName)) {
			std:: cerr << "error: can't create a temporary file (1)" << std:: endl;
			lis.discardOutputFile();
			return 2;
		}

		std:: vector<size_t> inputFileLengths;
		inputFileLengths.resize(inputFiles.size(), std::numeric_limits<size_t>::max());

		if (optionVerbose) {
			std:: cerr << "> detecting identical substrings" << std:: endl;
		}

		bool niceMode = false;
		size_t chunks = 0;
		bool chunkCountingDone = false;
		size_t progress = 0;
		if (optionDetectFrom == DETECT_WITHIN_FILE || optionDetectFrom == 0) {
			boost::scoped_ptr<PreprocessedFileReader> pPreprocessedFileReader(new PreprocessedFileReader());
			pPreprocessedFileReader->setParameterizationUsage(optionParameterization);
			pPreprocessedFileReader->setRawReader(rawReader);
			long processedTokens = 0;

			std::vector<ccfx_token_t> seqPrefetch;
			std::vector<size_t> fileLengthsPrefetch;
			size_t fiNext = 0;
			int rPrefetch = 0;

			size_t fi = 0;
			while (fi < selectedToInputTable.size()) {
				if (rPrefetch != 0) {
					lis.discardOutputFile();
					return rPrefetch;
				}

				std::vector<ccfx_token_t> seqFetched;
				std::vector<size_t> fileLengthsFetched;
				seqFetched.swap(seqPrefetch);
				fileLengthsFetched.swap(fileLengthsPrefetch);
#pragma omp parallel sections
				{
#pragma omp section
					if (fileLengthsFetched.size() > 0) {
						lis.setParens(pPreprocessedFileReader->refParens());
						lis.setPrefixes(pPreprocessedFileReader->refPrefixes());
						lis.setSuffixes(pPreprocessedFileReader->refSuffixes());

						size_t prevFileLengthTotal = 1;
						for (size_t c = 0; c < fileLengthsFetched.size(); ++c) {
							size_t fileLength = fileLengthsFetched[c];
							assert(fileLength < std::numeric_limits<size_t>::max());
							inputFileLengths[selectedToInputTable[fi]] = fileLength;
							if (fileLength >= optionB && (! lengthLimit || fileLength <= *lengthLimit)) {
								const InputFileData &fileFi = inputFiles[selectedToInputTable[fi]];

								seq.clear();
								seq.push_back(0); // head delimiter
								seq.insert(seq.end(), seqFetched.begin() + prevFileLengthTotal, seqFetched.begin() + prevFileLengthTotal + fileLength);

								fileStartPoss.clear();
								fileStartPoss.push_back(1);
								fileIDs.clear();
								fileIDs.push_back(fileFi.fileID);
								groupIDs.clear();
								groupIDs.push_back(fileFi.groupID);

								lis.setAllMode();
								//cd.setOptionVerbose(false);
								cd.attachSequence(&seq);
								if (optionDetectFrom != 0) {
									cd.findClonePair(&lis, hashFunc);
								}
							}
							++fi;
							prevFileLengthTotal += fileLength;
							processedTokens += fileLength;
						}
						assert(prevFileLengthTotal == seqFetched.size());
					}
#pragma omp section
					{
						rPrefetch = fetchPreprocessedFiles(&seqPrefetch, &fileLengthsPrefetch, 
							fiNext, 20, 
							selectedToInputTable, pPreprocessedFileReader.get(), 
							optionParameterUnification, chunkSize);
						fiNext += fileLengthsPrefetch.size();
					}
				} // end #pragma omp sections
				
				progress += fileLengthsFetched.size();
				progressRep.reportProgress(progress);
				if (processedTokens >= chunkSize) {
					pPreprocessedFileReader.reset(new PreprocessedFileReader());
					pPreprocessedFileReader->setParameterizationUsage(optionParameterization);
					pPreprocessedFileReader->setRawReader(rawReader);
					processedTokens = 0;
				}
			}
			assert(fiNext == selectedToInputTable.size());
			progressRep.reportDone();
			lis.writeEndOfCloneDataMark();
		}
		else {
			if (lengthLimit) {
				std::cerr << "error: option --prescreening requires option -w w+f-g-" << std::endl;
				return 2;
			}
			{
				bool only1chunk = false;
				size_t fi = 0;

				while (fi < selectedToInputTable.size()) {
					PreprocessedFileReader preprocessedFileReader;
					preprocessedFileReader.setRawReader(rawReader);
					preprocessedFileReader.setParameterizationUsage(optionParameterization);

					size_t fiStart = fi;

					fileStartPoss.clear();
					fileIDs.clear();
					groupIDs.clear();
					std::vector<size_t> fileLengths;
					{
						int r = fetchPreprocessedFiles(&seq, &fileLengths,
							fi, 0,
							selectedToInputTable, &preprocessedFileReader,
							optionParameterUnification, chunkSize);
						if (r != 0) {
							lis.discardOutputFile();
							return 2;
						}
					}

					{
						size_t prevFileLengthTotal = 1;
						for (size_t c = 0; c < fileLengths.size(); ++c) {
							const InputFileData &fileFi = inputFiles[selectedToInputTable[fi]];
							size_t fileLength = fileLengths[c];
							assert(fileLength < std::numeric_limits<size_t>::max());
							inputFileLengths[selectedToInputTable[fi]] = fileLength;
							fileStartPoss.push_back(prevFileLengthTotal);
							fileIDs.push_back(fileFi.fileID);
							groupIDs.push_back(fileFi.groupID);
							++fi;
							prevFileLengthTotal += fileLength;
						}
						assert(seq.size() == prevFileLengthTotal);
					}

					if (fi == selectedToInputTable.size()) {
						only1chunk = true;
						lis.setAllMode();
						lis.setParens(preprocessedFileReader.refParens());
						lis.setPrefixes(preprocessedFileReader.refPrefixes());
						lis.setSuffixes(preprocessedFileReader.refSuffixes());
						//cd.setOptionVerbose(optionVerbose);
						cd.attachSequence(&seq);
						cd.findClonePair(&lis, hashFunc);
						progressRep.reportDone();
					}
					else {
						size_t barriorPos = seq.size();
						size_t barriorFileSize = fileIDs.size();
						size_t gi = fi;

						std::vector<ccfx_token_t> seqPrefetch;
						std::vector<size_t> fileLengthsPrefetch;
						size_t nextGiPrefetch = gi;
						int rPrefetch = 0;

						while (gi < selectedToInputTable.size()) {
							if (rPrefetch != 0) {
								lis.discardOutputFile();
								return rPrefetch;
							}

							std::vector<ccfx_token_t> seqFetched;
							std::vector<size_t> fileLengthsFetched;
							seqFetched.swap(seqPrefetch);
							fileLengthsFetched.swap(fileLengthsPrefetch);
#pragma omp parallel sections
							{
#pragma omp section
								if (fileLengthsFetched.size() > 0) {
									if (! chunkCountingDone) {
										++chunks;
									}
									else {
										++progress;
									}
									size_t giStart = gi;
									seq.resize(barriorPos);
									fileStartPoss.resize(barriorFileSize);
									fileIDs.resize(barriorFileSize);
									groupIDs.resize(barriorFileSize);

									size_t prevFileLengthTotal = seq.size();
									for (size_t c = 0; c < fileLengthsFetched.size(); ++c) {
										const InputFileData &fileGi = inputFiles[selectedToInputTable[gi]];
										size_t fileLength = fileLengthsFetched[c];
										assert(fileLength < std::numeric_limits<size_t>::max());
										inputFileLengths[selectedToInputTable[gi]] = fileLength;
										fileStartPoss.push_back(prevFileLengthTotal);
										fileIDs.push_back(fileGi.fileID);
										groupIDs.push_back(fileGi.groupID);
										++gi;
										prevFileLengthTotal += fileLength;
									}
									seq.insert(seq.end(), seqFetched.begin() + 1, seqFetched.end());
									assert(seq.size() == prevFileLengthTotal);
									
									if (giStart == fi) {
										if (gi == selectedToInputTable.size()) {
											lis.setAllMode();
										}
										else {
											lis.setLeftAndCrossMode(barriorPos);
										}
									}
									else {
										lis.setCrossMode(barriorPos);
									}

									lis.setParens(preprocessedFileReader.refParens());
									lis.setPrefixes(preprocessedFileReader.refPrefixes());
									lis.setSuffixes(preprocessedFileReader.refSuffixes());
									cd.attachSequence(&seq);
									//std::cout << (boost::format("%d, %d, %d, %d") % fiStart % fi % giStart % gi) << std::endl; //debug
									cd.findClonePair(&lis, hashFunc);
									progressRep.reportProgress(progress);

									if (giStart == fi && gi == selectedToInputTable.size()) {
										fi = selectedToInputTable.size(); // break from "while (fi < selectedToInputTable.size())" loop
									}
								}
#pragma omp section
								if (nextGiPrefetch < selectedToInputTable.size()) {
									rPrefetch = fetchPreprocessedFiles(&seqPrefetch, &fileLengthsPrefetch, 
										nextGiPrefetch, 0, 
										selectedToInputTable, &preprocessedFileReader, 
										optionParameterUnification, chunkSize);
									nextGiPrefetch += fileLengthsPrefetch.size();
								}
							} // end #pragma omp sections
						}
						if (! chunkCountingDone) {
							chunkCountingDone = true;
							progressRep.setStartEnd(0, chunks * (chunks + 1) / 2);
							progress = chunks;
							if (optionVerbose) {
								std:: cerr << "> chunks of input files: " << (int)chunks << std:: endl;
							}
						}
					}
				}
				if (! only1chunk) {
					progressRep.reportDone();
				}
			}
			lis.writeEndOfCloneDataMark();
		}

		{
			PreprocessedFileReader preprocessedFileReader;
			preprocessedFileReader.setRawReader(rawReader);
			preprocessedFileReader.setParameterizationUsage(optionParameterization);
			for (size_t i = 0; i < selectedToInputTable.size(); ++i) {
				if (inputFileLengths[i] == std::numeric_limits<size_t>::max()) {
					std:: vector<ccfx_token_t> seq;
					seq.push_back(0); // head delimiter
					int r = readPreprocessedFile(&preprocessedFileReader, inputFiles[selectedToInputTable[i]].path, &seq);
					if (r != 0) {
						lis.discardOutputFile();
						return 2;
					}
					size_t fileLength = seq.size() - 1; // 1 for head delemiter
					inputFileLengths[i] = fileLength;
				}
			}
			lis.attachFileLengths(&inputFileLengths);
		}
		
		lis.closeOutputFile();
		
		boost::uint64_t detectedClones = lis.countClones();
		if (optionVerbose) {
			std:: cerr << "> count of detected clone pairs: " << detectedClones << std:: endl;
		}

		return 0;
	}
public:
	int main(const std::vector<std::string> &argv)
	{
		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std:: cout << 
				"Usage: ccfx D PREPROCESS_SCRIPT OPTIONS inputfiles..." "\n"
				"  Detects code clones from input files." "\n" 
				"  (Use \"ccfx F -p\" to obtain a list of available preprocess scripts.)" "\n"
				"Option" "\n"
				"  -b number: minimum length of a clone fragment. (50)" "\n"
				"  -c encoding: encoding of input files. (-c char)" "\n"
				"  -d directory: finds input files from the directory." "\n"
				"  -dn dir: shortcut for '-d dir -n dir'" "\n"
				"  -i listfile: list of input files." "\n"
				"  -ig fileid,...: makes a file group." "\n" 
				"  -j-: don't use majoritarian shaper." "\n"
				"  -k size: chunk size (60M)." "\n"
				"  -k-: on-memory detection. don't divide the input into chunks." "\n"
				"  -mr name: don't detect clones from files with the named remark." "\n"
				//"  -n-: do not access preprocess cache." "\n"
				"  -n dir: the directory where preprocessed files are created." "\n"
				"  -o out.ccfxd: outputs clone-data file name." "\n"
				"  -p: performs only preprocessing." "\n"
				"  -pp-: exact match. *experimental*" "\n"
				"  -r value: an option passed to preprocess script." "\n"
				"  -s number: =0 don't use block shaper, =1 easy, =2 soft, =3 hard. (2)" "\n"
				"  -s-: don't use block shaper." "\n"
				"  -t number: minimum size of token set of a code fragment. (12)" "\n"
				"  -u-: don't use p-match, which checks unification of parameters." "\n"
				"  -v: verbose option." "\n"
				"  -w params: detects within file/between files/between groups (-w w+f+g+)." "\n"
				"  --errorfiles=output: don't stop detection when syntax errors found. *experimental*" "\n"
				"  --prescreening=LEN.gt.num: don't detect clones from source files of length > num" "\n"
				"  --threads=number: max working threads (0)."
				;
			return 0;
		}

		{
			std::string errorMessage;
			int r = prepInvoker.read_script_table(theArgv0, &errorMessage);
			if (r != 0) {
				std::cerr << errorMessage;
				return r;
			}
		}

		int r = analyzeCommandLine(argv);
		if (r != 0) {
			return r;
		}
		if (inputFiles.empty()) {
			std:: cerr << "error: no input files are given" << std:: endl;
			return 1;
		}

		enum { IF_INVALID_NAME, IF_DOESNT_EXIST, IF_ISNT_A_FILE };
		typedef std::pair<size_t, int /* reason */> InvalidFileRecord;
		std::vector<InvalidFileRecord> invalidFiles;
		for (size_t i = 0; i < inputFiles.size(); ++i) {
			const std::string file = inputFiles[i].path;
			int fileID = inputFiles[i].fileID;
			if (! is_path_utf8_safe(file)) {
				invalidFiles.push_back(InvalidFileRecord(i, IF_INVALID_NAME));
			}
			else if (! path_exists(file)) {
				invalidFiles.push_back(InvalidFileRecord(i, IF_DOESNT_EXIST));
			}
			else if (! path_is_file(file)) {
				invalidFiles.push_back(InvalidFileRecord(i, IF_ISNT_A_FILE));
			}
		}

		if (! invalidFiles.empty()) {
			std::cerr << "error: invalid file found." << std::endl;
			boost::array<std::string, 3> messageTable = {{ "invalid name", "doesn't exist", "isn't a file" }};
			for (size_t ifi = 0; ifi < invalidFiles.size(); ++ifi) {
				const InvalidFileRecord &r = invalidFiles[ifi];
				std::cerr << (boost::format("  %s: file ID: %d, path: '%s'") 
						% messageTable[r.second] 
						% inputFiles[r.first].fileID
						% inputFiles[r.first].path) << std::endl;
			}
			return 1;
		}

		prepInvoker.setTemporaryDir(theTemporaryFileBaseName ? *theTemporaryFileBaseName : outputFileName);

		{
			if (outputFileName.empty()) {
				outputFileName = "a" + pairBinaryDiploidExtension;
				if (optionVerbose) {
					std:: cerr << "> output file name: " << outputFileName << std:: endl;
				}
			}

			force_extension(&outputFileName, pairBinaryDiploidExtension, ".tmp");
		}

		if (optionVerbose) {
			if (optionDetectFrom == DETECT_WITHIN_FILE || optionDetectFrom == 0) {
				std:: cerr << "> preprocessing/detecting identical substrings" << std:: endl;
			}
			else {
				std:: cerr << "> preprocessing" << std:: endl;
			}
		}
		{
			std::vector<std::string> fileNames;
			std::transform(inputFiles.begin(), inputFiles.end(), std::back_inserter(fileNames), boost::bind(&InputFileData::getPath, _1));
			r = prepInvoker.removeObsoletePreprocessedFiles(fileNames);
			if (r != 0) return r;
		}

		bool normalExecution = ! (optionOnlyPreprocess || optionDebugSortOnly);

		{
			std::vector<std::string> fileNames;
			std::transform(inputFiles.begin(), inputFiles.end(), std::back_inserter(fileNames), boost::bind(&InputFileData::getPath, _1));
			if (! optionParseErrors) {
				int r = prepInvoker.performPreprocess(fileNames);
				if (r != 0) return r;
			}
			else {
				std::vector<std::string> errorFileNames;
				int r = prepInvoker.performPreprocess(fileNames, &errorFileNames);
				if (r != 0) return r;

				if (! errorFileNames.empty()) {
					r = markErrorFiles(errorFileNames);
					if (r != 0) return r;
				}
			}
		}

		if (optionOnlyPreprocess) return 0;
		::my_sleep(::theWaitAfterProcessInvocation);

		if (optionDebugSortOnly) {
			if (inputFiles.size() != 1) {
				std:: cerr << "error: no clone data file" << std:: endl;
				return 1;
			}
			std:: string tempFileRaw = ::make_temp_file_on_the_same_directory(
					theTemporaryFileBaseName ? *theTemporaryFileBaseName : outputFileName, "ccfxrawclonedata", ".tmp");
			{
				FileStructWrapper p(inputFiles[0].path, "rb");
				if (! p) {
					std:: cerr << "error: can't open an input file '" << inputFiles[0].path << "'" << std:: endl;
					return 1;
				}
				FileStructWrapper q(tempFileRaw, "wb");
				if (! q) {
					std:: cerr << "error: can't create an temporary file '" << tempFileRaw << "'" << std:: endl;
					return 1;
				}
				while (true) {
					int ch = fgetc(p);
					if (ch == EOF) {
						break; // while
					}
					fputc(ch, q.getFileStruct());
				}
			}
			std:: cerr << "> sorting" << std:: endl;
			std:: string tempFileSorted = ::make_temp_file_on_the_same_directory(
					theTemporaryFileBaseName ? *theTemporaryFileBaseName : outputFileName, "ccfxsortedclonedata", ".tmp");
			RawClonePairFileTransformer sorter;
			sorter.setMemoryUsageLimit(chunkSize * 4);
			if (! sorter.sort(tempFileSorted, tempFileRaw)) {
				std:: cerr << sorter.getErrorMessage() << std:: endl;
				return 2;
			}
			remove(tempFileRaw.c_str());

			remove(outputFileName.c_str());
			r = rename(tempFileSorted.c_str(), outputFileName.c_str());
			if (r != 0) {
				std:: cerr << "error: can't create an output file '" << outputFileName << "'" << std:: endl;
				return 2;
			}
			return 0;
		}

		std:: string tempFileRaw = ::make_temp_file_on_the_same_directory(
				theTemporaryFileBaseName ? *theTemporaryFileBaseName : outputFileName, "ccfxrawclonedata", ".tmp");
		if (! maskRemarks.empty()) {
			std::vector<int> maskedFileIDs;
			for (std::map<int/* file ID */, std::vector<std::string> >::const_iterator i = inputFileRemarks.begin(); i != inputFileRemarks.end(); ++i) {
				int fileID = i->first;
				const std::vector<std::string> &rems = i->second;
				for (size_t j = 0; j < rems.size(); ++j) {
					const std::string &rem = rems[j];
					if (maskRemarks.find(rem) != maskRemarks.end()) {
						maskedFileIDs.push_back(fileID);
						break; // for j
					}
				}
			}
			r = detectClonesWithMask(tempFileRaw, maskedFileIDs);
		}
		else {
			r = detectClones(tempFileRaw);
		}

		if (r != 0) {
			return r;
		}
		r = do_sorting_and_shaping(tempFileRaw, outputFileName);
		if (r != 0) {
			return r;
		}

		if (optionVerbose) {
			std:: cerr << "> done." << std:: endl;
		}

		return 0;
	}
private:
	int markErrorFiles(const std::vector<std::string> &errorFileNames0) 
	{
		assert(!! optionParseErrors);
		std::vector<std::string> errorFileNames = errorFileNames0;

		if (*optionParseErrors == "-") {
			std::cerr << "info: error files:" << std::endl;
			for (std::vector<std::string>::const_iterator i = errorFileNames.begin(); i != errorFileNames.end(); ++i) {
				std::cerr << "  " << (*i) << std::endl;
			}
		}
		else {
			FileStructWrapper f(*optionParseErrors, "w");
			if (! f) {
				std::cerr << (boost::format("error: can't create a file: '%s'") % *optionParseErrors) << std::endl;
				return 2;
			}
			for (std::vector<std::string>::const_iterator i = errorFileNames.begin(); i != errorFileNames.end(); ++i) {
				const std::string &s = *i;
				FWRITEBYTES(s.data(), s.length(), f.getFileStruct());
			}
		}

		std::vector<int> errorFileIDs;

		std::sort(errorFileNames.begin(), errorFileNames.end());
		for (std::vector<InputFileData>::const_iterator i = inputFiles.begin(); i != inputFiles.end(); ++i) {
			const InputFileData &ifd = *i;
			std::vector<std::string>::const_iterator j = std::lower_bound(errorFileNames.begin(), errorFileNames.end(), ifd.path);
			if (j != errorFileNames.end() && *j == ifd.path) {
				errorFileIDs.push_back(ifd.fileID);
			} 
		}

		assert(! errorFileIDs.empty());
		if (! errorFileIDs.empty()) {
			const std::string preprocessError = "preprocess error";
			for (std::vector<int>::const_iterator i = errorFileIDs.begin(); i != errorFileIDs.end(); ++i) {
				int fileID = *i;
				inputFileRemarks[fileID].push_back(preprocessError);
			}
			maskRemarks.insert(preprocessError);
		}

		return 0;
	}
	int do_sorting_and_shaping(const std::string &tempFileRaw, const std::string &ofname)
	{
		if (optionDebugUnsort) {
			remove(ofname.c_str());
			int r = rename(tempFileRaw.c_str(), ofname.c_str());
			if (r != 0) {
				std:: cerr << "error: can't create an output file '" << ofname << "'" << std:: endl;
				return 2;
			}
			std:: cerr << "warning: the clone data file is unformal one. it use for debug purpose only" << std:: endl;
			return 0;
		}
		
		if (optionVerbose) {
			std:: cerr << "> sorting" << std:: endl;
		}
		std:: string tempFileSorted = ::make_temp_file_on_the_same_directory(
				theTemporaryFileBaseName ? *theTemporaryFileBaseName : ofname, "ccfxsortedclonedata", ".tmp");
		RawClonePairFileTransformer sorter;
		sorter.setMemoryUsageLimit(chunkSize * 4);
		if (! sorter.sort(tempFileSorted, tempFileRaw)) {
			std:: cerr << sorter.getErrorMessage() << std:: endl;
			return 2;
		}
		remove(tempFileRaw.c_str());

		std:: string t = tempFileSorted;
		if (optionShapingLevel >= 2) {
			std:: string tempFileShaped = ::make_temp_file_on_the_same_directory(
					theTemporaryFileBaseName ? *theTemporaryFileBaseName : ofname, "ccfxshapedclonedata", ".tmp");
			int r = TransformerMain::do_shaper(t, tempFileShaped, optionShapingLevel, true, optionVerbose);
			if (r != 0) {
				return r;
			}
			::remove(t.c_str());
			t = tempFileShaped;
		}
		else {
			std:: string tempFileIDTransformed = ::make_temp_file_on_the_same_directory(
					theTemporaryFileBaseName ? *theTemporaryFileBaseName : ofname, "ccfxshapedclonedata", ".tmp");
			int r = TransformerMain::do_id_transformation(t, tempFileIDTransformed, optionVerbose);
			if (r != 0) {
				return r;
			}
			::remove(t.c_str());
			t = tempFileIDTransformed;
		}

		if (optionMajoritarianShaper) {
			if (optionVerbose) {
				std::cerr << "> applying majoritarian shaper" << std::endl;
			}
			std:: string tempFileMajoritarianShaper = ::make_temp_file_on_the_same_directory(
					theTemporaryFileBaseName ? *theTemporaryFileBaseName : ofname, "ccfxcnclonedata", ".tmp");
			int r = TransformerMain::do_majoritarianShaper(t, tempFileMajoritarianShaper, optionVerbose,
					optionB / 2);
			if (r != 0) {
				return r;
			}
			::remove(t.c_str());
			t = tempFileMajoritarianShaper;
		}

		remove(ofname.c_str());
		int r = rename(t.c_str(), ofname.c_str());
		if (r != 0) {
			std:: cerr << "error: can't create an output file '" << ofname << "'" << std:: endl;
			return 2;
		}

		return 0;
	}
};

//long long strtoll(const std:: string &numstr)
//{
//	long long value = 0;
//	for (size_t i = 0; i < numstr.length(); ++i) {
//
//	}
//}

//long long strtoll(const char *str, char **p, int radix)
//{
//	assert(radix == 10);
//	long long value = 0;
//	size_t i;
//	for (i = 0; str[i] != '\0'; ++i) {
//		char ch = str[i];
//		if (! ('0' <= ch && ch <= '9')) {
//			*p = const_cast<char *>(str);
//			return 0; // invalid string
//		}
//		long long prev = value;
//		value *= 10;
//		value += ch - '0';
//		{
//			long long t = value;
//			t -= ch - '0';
//			t /= 10;
//			if (t != prev) {
//				*p = const_cast<char *>(str);
//				return 0; // invalid string
//			}
//		}
//	}
//
//	*p = const_cast<char *>(str) + i;
//	return value;
//}

//bool countLinesOfFile(
//		const std:: string &fileName, const std:: string &postfix,
//		PreprocessedFileReader *pPreprocessedFileReader, std:: string *pErrorMessage,
//		size_t *pLoc, // loc including comments or whitespaces
//		size_t *pSloc, // count of lines that contains tokens (comments will be excluded)
//		size_t *pLocOfAvailableTokens, // count of lines that contains specified tokens by availableTokens
//		const boost::dynamic_bitset<> *pAvailableTokens)
//{
//	PreprocessedFileReader &preprocessedFileReader = *pPreprocessedFileReader;
//	std:: string prepName = fileName + postfix;
//
//	if (! preprocessedFileReader.countLinesOfFile(prepName, postfix, pLoc, pSloc, pLocOfAvailableTokens, pAvailableTokens)) {
//		*pErrorMessage = std:: string("can't open a file '") + prepName + "'";
//		return false;
//	}
//
//	return true;
//}
//
static const char *splash = "%s ver. %s for %s (C) 2009-2010 AIST\n";

static const char *usage = 
	"ccfx [MODE] [-h] OTHER_PARAMETERS..." "\n"
	"Mode" "\n"
	//"  c: manipulate preprocess cache" "\n"
	"  d: detect code clones" "\n"
	"  f: find files" "\n"
	"  m: calculate metrics" "\n"
	"  p: pretty-print" "\n"
	//"  q: query" "\n"
	"  s: make subset of clone data" "\n"
	//"  t: transform clone data" "\n"
	"\n"
	"To see command line of each mode, put option -h after the mode char (e.g. ccfx f -h)"
	;


namespace data_size_check_for_persistence {
   BOOST_STATIC_ASSERT(sizeof(RawClonePair) == 32);
   //BOOST_STATIC_ASSERT(sizeof(long long) == 8);
}; // namespace

//bool data_size_check_for_persistence()
//{
//	size_t size = sizeof(RawClonePair);
//	if (size != 32) {
//		return false;
//	}
//
//	size_t sizeofLongLong = sizeof(long long);
//	if (sizeofLongLong != 8) {
//		return false;
//	}
//
//	return true;
//}

std:: vector<unsigned char> licenseData;
std:: string theUserID;
std:: string theExpireDate;
std:: string theArgv0;
boost::optional<std::string> theTemporaryFileBaseName;
int theWaitAfterProcessInvocation = 50;

std::string thePythonInterpreterPath;

void setup_global_constants(const char *argv0)
{
	Decoder defaultDecoder;

	theArgv0 = argv0;
	{
		boost::optional<std::string> s = getenvironmentvariable("CCFINDERX_EXECUTABLE_PATH");
		if (s) {
			theArgv0 = *s;
		}
	}

	{
		boost::optional<std::string> s = getenvironmentvariable("CCFINDERX_TEMPORARY_DIRECTORY");
		if (s) {
			if (path_is_relative(*s)) {
				theTemporaryFileBaseName = join_path(*s, "tmpfile");
			}
			else {
				theTemporaryFileBaseName = *s;
			}
		}
		//std::cerr << "debug 3: theTemporaryFileBaseName = " << theTemporaryFileBaseName << std::endl;
	}

	{
		boost::optional<std::string> s = getenvironmentvariable("CCFINDERX_PYTHON_INTERPRETER_PATH");
		if (s) {
			//std::cout << "debug: CCFINDERX_PYTHON_INTERPRETER_PATH=" << (*s) << std::endl;
			thePythonInterpreterPath = *s;
			if (thePythonInterpreterPath.length() > 0 && thePythonInterpreterPath[0] != '\"' && thePythonInterpreterPath.find(' ') != std::string::npos) {
				thePythonInterpreterPath = '\"' + thePythonInterpreterPath + '\"';
			}
		}
		else {
#if defined OS_UBUNTU
			thePythonInterpreterPath = "/usr/bin/python";
#endif
#if defined _MSC_VER
			std::string s = get_open_command_for_extension(".py");
			if (s.length() > 0) {
				if (s[0] == '\"') {
					std::string::size_type endPos = s.find('\"', 1);
					if (endPos != std::string::npos) {
						s = s.substr(1, endPos - 1);
					}
				}
				else {
					std::string::size_type endPos = s.find(' ');
					if (endPos != std::string::npos) {
						s = s.substr(endPos);
					}
				}
			}
			thePythonInterpreterPath = s;
#endif
		}
		if (thePythonInterpreterPath.length() == 0) {
			std::cerr << "error: the python interpreter not found" << std::endl;
			std::cerr << "  (set environment variable CCFINDERX_PYTHON_INTERPRETER_PATH)" << std::endl;
			exit(1);
			return;
		}
	}
}

const std:: string LICENSE5 =  "xpiclk";

int expand_command_file(std::vector<std::string> *pArgvec)
{
	const std::vector<std::string> &argvec = *pArgvec;
	std::vector<std::string> result;

	for (size_t i = 0; i < argvec.size(); ++i) {
		const std::string &argi = argvec[i];
		if (argi == "-@") {
			if (! (i + 1 < argvec.size())) {
				std::cerr << "error: -@ requires an argument" << std::endl;
				return 1;
			}
			std::string filename = argvec[i + 1];
			std::vector<std::string> lines;
			if (! get_raw_lines(filename, &lines)) {
				std::cerr << "error: can't open file '" << filename << "'" << std::endl;
				return 1;
			}
			for (size_t j = 0; j < lines.size(); ++j) {
				const std::string &line = lines[j];
				boost::optional<std::pair<std::string, std::string> > pOptionLineValues;
				if (! line.empty() && line[0] == '-') {
					size_t k = 1;
					while (k < line.size()) {
						int ch = line[k];
						if (ch == ' ' || ch== '\t') {
							pOptionLineValues = std::pair<std::string, std::string>(line.substr(0, k), line.substr(k + 1));
							break; // while k
						}
						if (! ('a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_' || ch == '-')) {
							break; // while k
						}
						++k;
					}
				}
				if (pOptionLineValues) {
					std::pair<std::string, std::string> optionLineValue = *pOptionLineValues;
					result.push_back(optionLineValue.first);
					result.push_back(optionLineValue.second);
				}
				else {
					result.push_back(line);
				}
			}
			++i;
		}
		else {
			result.push_back(argi);
		}
	}

	(*pArgvec).swap(result);

	return 0;
}

#if defined _MSC_VER
std::string get_module_file_name()
{
	std::vector<char> buf;
	buf.resize(_MAX_PATH * 4);
	while (true) {
		DWORD resultCodeOfGetModuleFileName = ::GetModuleFileName(NULL, &buf[0], buf.size());
		assert(resultCodeOfGetModuleFileName != 0);
		size_t len = resultCodeOfGetModuleFileName;
		if (len < buf.size() - 2) {
			return std::string(&buf[0]);
		}
		buf.resize(buf.size() * 2);
	}
}
#endif

int main(int argc, char *argv[])
{
	::setlocale(LC_ALL, "");

	//if (! data_size_check_for_persistence()) {
	//	std:: cerr << "error: in self check" << std:: endl;
	//	return 3;
	//}

	std::string argv0 = argv[0];
#if defined _MSC_VER
	{
		argv0 = get_module_file_name();
		if (argv0.find(' ') != std::string::npos) {
			boost::optional<std::string> s = get_short_path_name(argv0);
			if (! s) {
				std::cerr << "error: in invocation of ::GetShortPathName()" << std::endl;
				return 1;
			}
			argv0 = *s;
		}
	}
#endif
	setup_global_constants(argv0.c_str());

	std:: string versionStr = (boost::format("%ld.%ld.%ld.%ld") % APPVERSION[0] % APPVERSION[1] % APPVERSION[2] % APPVERSION[3]).str();

	if (argc == 1) {
		std:: cout << (boost::format(splash) % CCFINDERX % versionStr % PLATFORM_NAME);
		return 0;
	}

	std::vector<std::string> argvec;
	std::for_each(argv, argv + argc, boost::bind(&std::vector<std::string>::push_back, &argvec, _1));
	int r = expand_command_file(&argvec);
	if (r != 0) {
		return r;
	}

	if (argvec.size() == 1 || argvec[1] == "-h" || argvec[1] == "--help") {
		std:: cout << usage;
		return 0;
	}

	if (argvec[1] == "DB" || argvec[1] == "db") {
		std:: cerr << "error: can't open the license file" << std:: endl;
		std:: cerr << "error: CCFinderX license is expired" << std:: endl;
		return 1;
	}
	//else if (argvec[1] == "C" || argvec[1] == "c") {
	//	return PreprocessCacheManipulateMain().main(argc, argv);
	//}
	else if (argvec[1] == "F" || argvec[1] == "f") {
		return FindFileMain().main(argvec);
	}
	else if (argvec[1] == "P" || argvec[1] == "p") {
		return PrettyPrintMain().main(argvec);
	}
	else if (argvec[1] == "S" || argvec[1] == "s") {
		return FilteringMain().main(argvec);
	}
	else if (argvec[1] == "M" || argvec[1] == "m") {
		return MetricMain().main(argvec);
	}
	else if (argvec[1] == "Q" || argvec[1] == "q") {
		return QueryMain().main(argvec);
	}
	//else if (argvec[1] == "T" || argvec[1] == "t") {
	//	return TransformerMain().main(argvec);
	//}
	else if (argvec[1] == "D" || argvec[1] == "d") {
		return CloneDetectionMain().main(argvec);
	}
	else {
		std::cout << "error: no mode specified" << std::endl;
		return 1;
	}
}

