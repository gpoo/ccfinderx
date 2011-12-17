#include <cassert>
#include <cstdlib>
#include <map>
#include <hash_map>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../../common/utf8support.h"

inline MYWCHAR_T to_compared(const std::vector<MYWCHAR_T> &seq, size_t pos, size_t begin)
{
	return seq[pos];
}

inline MYWCHAR_T to_reversereference_compared(const std::vector<MYWCHAR_T> &seq, size_t pos, size_t begin, size_t end)
{
	return seq[pos];
}

#include "../clonedetector.h"

class ProgressReporter {
private:
	std:: ostream *pOutput;
	int progressDone;
	int progressReportResolution;
	int startValue;
	int endValue;
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
	void setStartEnd(int startValue_, int endValue_)
	{
		assert(endValue_ >= startValue_);
		startValue = startValue_;
		endValue = endValue_;
	}
	void reportProgress(int curValue)
	{
		if (pOutput == NULL) {
			return;
		}

		// check for avoiding zero-divide error
		if (startValue >= endValue) {
			return;
		}

		int p = (curValue - startValue) * progressReportResolution / (endValue - startValue);
		while (progressDone < p) {
			(*pOutput) << (progressDone % 2 == 0 ? "." : "\b:");
			++progressDone;
			if (progressDone % (progressReportResolution/4) == 0) {
				(*pOutput) << (100 * progressDone / progressReportResolution) << "%";
			}
		}
	}
	void reportDone()
	{
		reportProgress(endValue);
	}
};

bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines)
{
	std:: ifstream is;
	is.open(file_path.c_str(), std:: ios::in | std:: ios::binary);
	if (! is.is_open()) {
		return false;
	}
	
	std:: vector<std:: string> lines;
	std:: string str;
	while (! is.eof()) {
		std:: getline(is, str, '\n');
		if (str.length() >= 1 && str[str.length() - 1] == '\r') {
			str.resize(str.length() - 1);
		}
		lines.push_back(str);
	}
	is.close();
	if (lines.size() >= 1 && lines[lines.size() - 1].length() == 0) {
		lines.resize(lines.size() - 1);
	}

	(*pLines).swap(lines);

	return true;	
}

class MySequenceHashFunction : public CloneDetector<MYWCHAR_T, unsigned short>::SequenceHashFunction {
public:
	virtual unsigned short operator()(const std:: vector<MYWCHAR_T> &seq, size_t begin, size_t end)
	{
		unsigned short hashValue = 0;
		int j = 0;
		for (std:: vector<MYWCHAR_T>::const_iterator i = seq.begin() + begin; i != seq.begin() + end; ++i) {
			hashValue += (unsigned short)(*i) * (*i);
			//hashValue += (j + 1) * (unsigned short)(*i);
			//hashValue += (1 + 5 * j) * ((unsigned short)(*i));
			//hashValue *= 2; hashValue += (unsigned short)*i;
			++j;
		}
		return hashValue;
	}
};

class FilePosClonePairListener : public CloneDetector<MYWCHAR_T, unsigned short>::ClonePairListenerWithScope {
private:
	const std:: vector<std:: string> *pInputFiles;
	const std:: vector<size_t> *pFileStartPoss;
	const std:: vector<std:: vector<size_t> > *pLineStartPoss;
	size_t targetLength;
	std:: ostream *pOutput;
public:
	FilePosClonePairListener()
		: pInputFiles(NULL), pFileStartPoss(NULL), pLineStartPoss(NULL), targetLength(0), 
				pOutput(&std:: cout)
	{
	}
	void attachFileNames(const std:: vector<std:: string> *pInputFiles_)
	{
		pInputFiles = pInputFiles_;
	}
	void attachFileStartPositions(const std:: vector<size_t> *pFileStartPoss_)
	{
		pFileStartPoss = pFileStartPoss_;
	}
	void attachLineStartPositions(const std:: vector<std:: vector<size_t> > *pLineStartPoss_)
	{
		pLineStartPoss = pLineStartPoss_;
	}
	void setTargetLength(size_t targetLength_)
	{
		targetLength = targetLength_;
	}
	void attachOutput(std:: ostream *pOutput_)
	{
		pOutput = pOutput_;
	}
private:
	void getFileNameRowAndPosOfPos(size_t posA, const std:: string **ppFileName, std:: pair<size_t, size_t> *pRowCol, size_t *posARelative)
	{
		const std:: vector<std:: string> &inputFiles = *pInputFiles;
		const std:: vector<size_t> &fileStartPoss = *pFileStartPoss;
		const std:: vector<std:: vector<size_t> > &lineStartPoss = *pLineStartPoss;
		assert(inputFiles.size() == fileStartPoss.size());
		assert(inputFiles.size() == lineStartPoss.size());
		
		std:: vector<size_t>::const_iterator posAFile = std:: upper_bound(fileStartPoss.begin(), fileStartPoss.end(), posA);
		--posAFile;
		size_t posAFileIndex = posAFile - fileStartPoss.begin();
		assert(posAFileIndex < inputFiles.size());
		if (ppFileName != NULL) {
			*ppFileName = &inputFiles[posAFileIndex];
		}

		const std:: vector<size_t> &lineStarts = lineStartPoss[posAFileIndex];
		if (lineStarts.size() == 0) {
			std:: cerr << "posAFileIndex = " << posAFileIndex << std:: endl;
		}
		assert(lineStarts.size() >= 1);
		*posARelative = posA - lineStarts[0];

		if (pRowCol != NULL) {
			assert(posAFileIndex < lineStartPoss.size());
			std:: vector<size_t>::const_iterator posALine = std:: upper_bound(lineStarts.begin(), lineStarts.end(), posA);
			--posALine;
			std:: vector<size_t>::const_iterator posAFileLine = std:: upper_bound(lineStarts.begin(), lineStarts.end(), (*posAFile));
			--posAFileLine;
			std::pair<size_t, size_t> posARowCol(posALine - posAFileLine, posA - (*posALine));
			const std:: vector<MYWCHAR_T> &seq = refSeq();
			if (seq[posA] == '\n') { // その位置が改行だったときは、次の行の頭をその位置とする
				++posARowCol.first;
				posARowCol.second = 0;
			}
			(*pRowCol).swap(posARowCol);
		}
	}
public:
	void found_scoped(size_t posA, size_t posB, size_t baseLength, unsigned long long int cloneSetReferenceNumber)
	{
		assert(posA < posB);

		size_t length = baseLength;

		if (length < targetLength) {
			return;
		}

		const std:: string *pPosAFileName;
		std:: pair<size_t, size_t> posABeginRowCol;
		std:: pair<size_t, size_t> posAEndRowCol;
		size_t posABeginR;
		size_t posAEndR;
		getFileNameRowAndPosOfPos(posA, &pPosAFileName, &posABeginRowCol, &posABeginR);
		getFileNameRowAndPosOfPos(posA + length, NULL, &posAEndRowCol, &posAEndR);
		assert(posAEndR - posABeginR == length);
		
		const std:: string *pPosBFileName;
		std:: pair<size_t, size_t> posBBeginRowCol;
		std:: pair<size_t, size_t> posBEndRowCol;
		size_t posBBeginR;
		size_t posBEndR;
		getFileNameRowAndPosOfPos(posB, &pPosBFileName, &posBBeginRowCol, &posBBeginR);
		getFileNameRowAndPosOfPos(posB + length, NULL, &posBEndRowCol, &posBEndR);
		assert(posBEndR - posBBeginR == length);

		(*pOutput) << cloneSetReferenceNumber << "\t";
		(*pOutput) << (*pPosAFileName) << "\t" << (posABeginRowCol.first + 1) << "." << (posABeginRowCol.second + 1) << "." << posABeginR << "\t";
		(*pOutput) << (posAEndRowCol.first + 1) << "." << (posAEndRowCol.second + 1) << "." << posAEndR << "\t";
		(*pOutput) << (*pPosBFileName) << "\t" << (posBBeginRowCol.first + 1) << "." << (posBBeginRowCol.second + 1) << "." << posBBeginR << "\t";
		(*pOutput) << (posBEndRowCol.first + 1) << "." << (posBEndRowCol.second + 1) << "." << posBEndR << std:: endl;
	}
};

class EncodedFileReader //: public FileReader
{
private:
	std:: string errorMessage;
	Decoder decoder;
public:
	bool setEncoding(const std:: string &encoding)
	{
		return decoder.setEncoding(encoding);
	}
	bool read(std:: vector<MYWCHAR_T> *pSeq, const std:: string &fileName)
	{
		std:: ifstream input;
		input.open(fileName.c_str(), std:: ios::in | std:: ios::binary);
		if (! input.is_open()) {
			errorMessage = "fail to open file '" + fileName + "'";
			return false;
		}
		
		input.seekg(0, std:: ios::end);
		size_t inputSize = input.tellg();
		input.seekg(0, std:: ios::beg);
		
		std:: vector<char> buf;
		buf.resize(inputSize);
		input.read(&buf[0], inputSize);
		input.close();

		decoder.decode(pSeq, &buf[0], &buf[0] + buf.size());
		return true;
	}
	std:: string getErrorMessage() const
	{
		return errorMessage;
	}
};

bool read_file(const std:: string &fileName, 
		std:: vector<MYWCHAR_T> *pSeq,
		std:: vector<std:: string> *pFileNames,
		std:: vector<size_t> *pFileStartPoss,
		std:: vector<std:: vector<size_t> > *pLineStartPoss,
		EncodedFileReader *pEncodedFileReader)
{
	std:: vector<MYWCHAR_T> &seq = *pSeq;
	std:: vector<std:: string> &fileNames = *pFileNames;
	std:: vector<size_t> &fileStartPoss = *pFileStartPoss;
	std:: vector<std:: vector<size_t> > &lineStartPoss = *pLineStartPoss;
	
	assert(seq.size() > 0 && seq[seq.size() - 1] == 0); // check delimiter is pushed

	fileNames.push_back(fileName);
	fileStartPoss.push_back(seq.size());
	std:: vector<size_t> lineStarts;
	std:: vector<std:: string> lines;
	if (! get_raw_lines(fileName, &lines)) {
		return false;
	}
	lineStarts.reserve(lines.size() + 1);
	for (size_t li = 0; li < lines.size(); ++li) {
		lineStarts.push_back(seq.size());
		seq.insert(seq.end(), lines[li].begin(), lines[li].end());
		seq.push_back('\n');
	}
	lineStarts.push_back(seq.size());
	lineStartPoss.resize(lineStartPoss.size() + 1);
	assert(lineStarts.size() > 0);
	lineStartPoss[lineStartPoss.size() - 1].swap(lineStarts);
	seq.push_back(0); // delimiter

	return true;
}

std:: pair<int/* progress */, int/* total */> calc_progress(int s, int fbegin, int fend, int g)
{
	//std:: cerr << s << "," << fbegin << "," << fend << "," << g << std::endl;
	int total = s;
	int progress = fend + (fend - fbegin) * (g - fend) / (s - fend);
	return std:: pair<int, int>(progress, total);
}

int main(int argc, char *argv[])
{
	const char *usage = "Duplicated Text Finder ver. 1.0 for WinXP (C) 2009-2010 AIST" "\n" "\n"
		"Usage: dtfind OPTIONS inputfiles..." "\n"
		"Option" "\n"
		"-b number: minimum length of a clone fragment. (1000)" "\n"
		"-i listfile: list file of input files." "\n" 
		"-m size: chunk size (60M)." "\n"
		"-m -: specify to perfrom the detection on memory, without deviding the inputs into chunks." "\n"
		"-o outputfile: output file name." "\n"
		"-v: verbose option." "\n";

	std:: vector<std:: string> inputFiles;
	std:: string outputFileName;
	bool optionVerbose = false;

	int optionB = 1000;
	size_t chunkSize = 60 * 1024 * 1024;

	if (argc == 1) {
		std:: cout << usage;
		return 0;
	}
	{
		for (size_t i = 1; i < argc; ++i) {
			std:: string argi = argv[i];
			if (argi == "-h" || argi == "--help" || argi == "/?") {
				std:: cout << usage;
				return 0;
			}
			else if (argi == "-b") {
				if (i + 1 < argc) {
					int b;
					if (sscanf(argv[i + 1], "%d", &b) == 1) {
						optionB = b;
					}
					else {
						std:: cerr << "error: invalid argument is given to option -b" << std:: endl;
						return 1;
					}
					++i;
				}
				else {
					std:: cerr << "error: option -b requres an argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-i") {
				if (i + 1 < argc) {
					std:: string listFileName = argv[i + 1];
					if (! get_raw_lines(listFileName, &inputFiles)) {
						std:: cerr << "error: can not open a file '" << listFileName << "'" << std:: endl;
						return 1;
					}
					++i;
				}
				else {
					std:: cerr << "error: option -i requres an argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-m") {
				if (i + 1 < argc) {
					std:: string sizeStr = argv[i + 1];
					if (sizeStr == "-") {
						chunkSize = 0;
					}
					else if (sizeStr.length() >= 1) {
						int lastCh = sizeStr[sizeStr.length() - 1];
						if (lastCh == 'b' || lastCh == 'B') {
							sizeStr = sizeStr.substr(0, sizeStr.length() - 1);
							if (sizeStr.length() >= 1) {
								lastCh = sizeStr[sizeStr.length() - 1];
							}
							else {
								std:: cerr << "error: invalid -m argument" << std:: endl;
								return 1;
							}
						}
						size_t factor = 1;
						switch (lastCh) {
						case 'k': case 'K':
							factor = 1024;
							sizeStr = sizeStr.substr(0, sizeStr.length() - 1);
							break;
						case 'm': case 'M':
							factor = 1024 * 1024;
							sizeStr = sizeStr.substr(0, sizeStr.length() - 1);
							break;
						default:
							if (! ('0' <= lastCh && lastCh <= '9')) {
								std:: cerr << "error: invalid -m argument" << std:: endl;
								return 1;
							}
						}
						long siz;
						if (sscanf(sizeStr.c_str(), "%ld", &siz) != 1) {
							std:: cerr << "error: invalid -m argument" << std:: endl;
							return 1;
						}
						chunkSize = ((size_t)siz) * factor;
					}
					++i;
				}
				else {
					std:: cerr << "error: option -m requres an argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-o") {
				if (i + 1 < argc) {
					outputFileName = argv[i + 1];
					++i;
				}
				else {
					std:: cerr << "error: option -o requres an argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-v") {
				optionVerbose = true;
			}
			else {
				inputFiles.push_back(argi);						
			}
		}
	}
	if (inputFiles.size() == 0) {
		std:: cerr << "error: no input files are given" << std:: endl;
	}

	size_t bottomUnitLength = 25;
	size_t multiply = 1;
	if (optionB < bottomUnitLength) {
		bottomUnitLength = optionB;
	}
	else {
		multiply = optionB / bottomUnitLength;
		assert(multiply >= 1);
		assert(multiply * bottomUnitLength <= optionB);
	}
	
	CloneDetector<char, unsigned short> cd;
	MySequenceHashFunction hashFunc;

	cd.setBottomUnitLength(bottomUnitLength);
	cd.setMultiply(multiply);
	size_t unitLength = cd.getUnitLength();
	assert(unitLength <= optionB);
	
	std:: ostream *pOutput = &std:: cout;
	if (outputFileName.length() > 0) {
		pOutput = new std:: ofstream(outputFileName.c_str(), std:: ios::out);
		if (! (*pOutput)) {
			std:: cerr << "error: can not create a file '" << outputFileName << "'" << std:: endl;
			return 1;
		}
	}
	
	(*pOutput) << "#version: dtfind 1.0" << std:: endl;
	(*pOutput) << "#option: -b " << optionB << std:: endl;
	(*pOutput) << "#fields: SetID\tLFile\tLLine.LColumn.LIndex\tRFile\tRLine.LColumn.RIndex" << std:: endl;
	ProgressReporter progressRep;
	std:: pair<int, int> t = calc_progress(inputFiles.size(), 0, 0, 0);
	progressRep.setStartEnd(0, t.second);
	if (optionVerbose) {
		progressRep.attachOutput(&std:: cerr);
	}

	std:: vector<char> seq;
	std:: vector<std:: string> fileNames;
	std:: vector<size_t> fileStartPoss;
	std:: vector<std:: vector<size_t> > lineStartPoss;
	fileNames.reserve(inputFiles.size());
	fileStartPoss.reserve(inputFiles.size());
	lineStartPoss.reserve(inputFiles.size());
	
	FilePosClonePairListener lis;
	lis.attachOutput(pOutput);
	lis.setTargetLength(optionB);
	lis.attachFileNames(&fileNames);
	lis.attachFileStartPositions(&fileStartPoss);
	lis.attachLineStartPositions(&lineStartPoss);
	size_t chunks = 0;
	bool chunkCountingDone = false;
	size_t progress = 0;
	if (optionVerbose) {
		if (chunkSize > 0) {
			std:: cerr << ">counting chunks" << std:: endl;
		}
	}
	{
		bool only1chunk = false;
		size_t fi = 0;
		while (fi < inputFiles.size()) {
			size_t fiStart = fi;
			fileNames.clear();
			seq.clear();
			fileStartPoss.clear();
			lineStartPoss.clear();

			seq.push_back(0); // head delimiter
			while (fi < inputFiles.size() && (chunkSize == 0 || seq.size() - 1/* for head delimiter */ < chunkSize)) {
				if (! read_file(inputFiles[fi], &seq, &fileNames, &fileStartPoss, &lineStartPoss)) {
					std:: cerr << "error: can not open a file '" << inputFiles[fi] << "'" << std:: endl;
					return 2;
				}
				++fi;
			}
			
			if (fi == inputFiles.size()) {
				only1chunk = true;
				lis.setAllMode();
				cd.setOptionVerbose(optionVerbose);
				cd.attachSequence(&seq);
				cd.findClonePair(&lis, hashFunc);
			}
			else {
				size_t barriorPos = seq.size();
				size_t barriorFileSize = fileNames.size();
				size_t gi = fi;
				while (gi < inputFiles.size()) {
					if (! chunkCountingDone) {
						++chunks;
					}
					else {
						++progress;
					}
					size_t giStart = gi;
					seq.resize(barriorPos);
					fileNames.resize(barriorFileSize);
					fileStartPoss.resize(barriorFileSize);
					lineStartPoss.resize(barriorFileSize);

					while (gi < inputFiles.size() && seq.size() - barriorPos < chunkSize) {
						if (! read_file(inputFiles[gi], &seq, &fileNames, &fileStartPoss, &lineStartPoss)) {
							std:: cerr << "error: can not open a file '" << inputFiles[gi] << "'" << std:: endl;
							return 2;
						}
						++gi;
					}
					
					if (giStart == fi) {
						if (gi == inputFiles.size()) {
							lis.setAllMode();
						}
						else {
							lis.setLeftAndCrossMode(barriorPos);
						}
					}
					else {
						lis.setCrossMode(barriorPos);
					}
					cd.attachSequence(&seq);
					cd.findClonePair(&lis, hashFunc);
					progressRep.reportProgress(progress);
					if (giStart == fi) {
						if (gi == inputFiles.size()) {
							fi = gi;
						}
					}
				}
				if (! chunkCountingDone) {
					chunkCountingDone = true;
					progressRep.setStartEnd(0, chunks * (chunks + 1) / 2);
					progress = chunks;
				}
			}
		}
		if (! only1chunk) {
			progressRep.reportDone();
		}
	}

	if (pOutput != &std:: cout) {
		delete pOutput;
	}

	return 0;
}
