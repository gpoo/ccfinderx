#if ! defined PRETTYPRINTMAIN_H
#define PRETTYPRINTMAIN_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "../common/utf8support.h"
#include "../common/unportable.h"
#include "ccfxconstants.h"
#include "ccfxcommon.h"
#include "rawclonepairdata.h"

class PrettyPrintMain {
private:
	std:: string inputFile;
	std:: string ext;
	std:: string outputFile;
	Decoder defaultDecoder;

	std::string usage;
	std::vector<std::pair<std::string /* option name */, std::string /* comment */> > optionDefinitions;

private:
	std:: string SYS2INNER(const std::string &systemString)
	{
		return toUTF8String(defaultDecoder.decode(systemString));
	}
	std:: string INNER2SYS(const std::string &innerString)
	{
		return defaultDecoder.encode(toWStringV(innerString));
	}
private:
	bool assemble(const std::string &outputFile, const std::string &inputFile, std::string *pErrorMessage);
public:
	PrettyPrintMain()
	{
		usage = "Usage 1: ccfx P OPTIONS inputfile [-o outputfile]" "\n"
				"  Prints a clone-data file(*.ccfxd) or a preprocessed file(*.ccfxprep)" "\n"
				"  in human-readable format." "\n"
				"Option" "\n"
				"  -l: extracts file list" "\n"
				"  -ln: extracts file list with file ID" "\n"
				"  -lnr: extracts file list with file ID and remark" "\n"
				"  -lr: extracts file list with remark" "\n"
				"  -r: extracts source-file remarks" "\n"
				"  -v: prints version of ccfx which generated inputfile" "\n"
				"Usage 2: ccfx P -a inputfile -o outputfile" "\n"
				"  Converts (assemble) a pretty-printed clone data into binary format (*.ccfxd)." "\n"
				;
		
		struct { const char *name; const char *comment; } OPTION_NAMES[] = { 
			{ "-b", "minimumCloneLength" },
			{ "-s", "shapingLevel" },
			{ "-u", "useParameterUnification" },
			{ "-t", "minimumTokenSetSize" },
			{ NULL, NULL }
		};
		std::vector<std::pair<std::string /* option name */, std::string /* comment */> > optionDefinitions;
		for (size_t i = 0; OPTION_NAMES[i].name != NULL; ++i) {
			optionDefinitions.push_back(std::pair<std::string /* option name */, std::string /* comment */>(OPTION_NAMES[i].name, OPTION_NAMES[i].comment));
		}
	}
public:
	int main(const std::vector<std::string> &argv) 
	{
		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std::cout << usage;
			return 0;
		}

		int fileID;
		bool fileIDSpecified = false;
		boost::uint64_t cloneID;
		bool cloneIDSpecified = false;
		enum { FLF_FilePath = 1 << 0, FLF_FileID = 1 << 1, FLF_Remark = 1 << 2 };
		int optionFileListFields = 0;
		bool optionCheckVersion = false;
		bool optionAssemble = false;

		for (size_t i = 2; i < argv.size(); ++i) {
			std:: string argi = argv[i];

			if (! argi.empty() && argi[0] == '-') {
				if (argi == "-o") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -o requires an argument" << std:: endl;
						return 1;
					}
					outputFile = argv[i + 1];
					++i;
				}
				else if (argi == "-f") {
					if (fileIDSpecified == true) {
						std:: cerr << "error: option -f specified twice" << std:: endl;
						return 1;
					}
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -f requires an argument" << std:: endl;
						return 1;
					}
					std:: string argi2 = argv[i + 1];
					try {
						fileID = boost::lexical_cast<int>(argi2);
					}
					catch (boost::bad_lexical_cast &) {
						std:: cerr << "error: invalid -f argument" << std:: endl;
						return 1;
					}
					fileIDSpecified = true;
				}
				else if (argi == "-c") {
					if (cloneIDSpecified == true) {
						std:: cerr << "error: option -c specified twice" << std:: endl;
						return 1;
					}
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -c requires an argument" << std:: endl;
						return 1;
					}
					std:: string argi2 = argv[i + 1];
					try {
						cloneID = boost::lexical_cast<boost::uint64_t>(argi2);
					}
					catch (boost::bad_lexical_cast &) {
						std:: cerr << "error: invalid -c argument" << std:: endl;
						return 1;
					}
					cloneIDSpecified = true;
				}
				else if (argi == "-l") {
					optionFileListFields |= FLF_FilePath;
				}
				else if (argi == "-ln") {
					optionFileListFields |= FLF_FilePath | FLF_FileID;
				}
				else if (argi == "-lnr") {
					optionFileListFields |= FLF_FilePath | FLF_FileID | FLF_Remark;
				}
				else if (argi == "-lr") {
					optionFileListFields |= FLF_FilePath | FLF_Remark;
				}
				else if (argi == "-r") {
					optionFileListFields |= FLF_Remark;
				}
				else if (argi == "-v") {
					optionCheckVersion = true;
				}
				else if (argi == "-a") {
					optionAssemble = true;
				}
				else {
					std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (inputFile.empty()) {
					inputFile = argi;
				}
				else {
					std:: cerr << "error: too many command-line arguments" << std:: endl;
					return 1;
				}
			}
		}

		switch (optionFileListFields) {
		case FLF_FilePath:
		case FLF_FilePath | FLF_FileID:
		case FLF_FilePath | FLF_Remark:
		case FLF_FilePath | FLF_FileID | FLF_Remark:
			break;
		case FLF_FileID:
		case FLF_FileID | FLF_Remark:
			assert(false); // internal error
			break;
		case FLF_Remark:
		case 0:
			break;
		}

		if (optionAssemble) {
			if (inputFile.empty()) {
				std:: cerr << "error: file not given" << std:: endl;
				return 1;
			}
			if (outputFile.empty()) {
				std:: cerr << "error: option -o required" << std:: endl;
				return 1;
			}
			if (boost::algorithm::ends_with(inputFile, ".ccfxd") || boost::algorithm::ends_with(inputFile, ".ccfxd.tmp")) {
				std::cerr << "error: the input file seems clone data file." << std::endl;
				return 1;
			}

			splitpath(outputFile, NULL, NULL, &ext);
			if (ext == ".tmp") {
				std:: string s = outputFile.substr(0, outputFile.length() - std:: string(".tmp").length());
				splitpath(s, NULL, NULL, &ext);
			}

			if (ext != ".ccfxd") {
				outputFile += ".ccfxd";
			}

			std::string errorMessage;
			if (! assemble(outputFile, inputFile, &errorMessage)) {
				std::cerr << "error: " << errorMessage << std::endl;
				return 1;
			}

			return 0;
		}
		else {
			if (inputFile.empty()) {
				std:: cerr << "error: file not given" << std:: endl;
				return 1;
			}
			splitpath(inputFile, NULL, NULL, &ext);
			if (ext == ".tmp") {
				std:: string s = inputFile.substr(0, inputFile.length() - std:: string(".tmp").length());
				splitpath(s, NULL, NULL, &ext);
			}

			if (! (ext == ".ccfxd" || ext == ".icet" || ext == ".ccfxprep")) {
				std:: cerr << "error: file is neither clone-data file or preprocessed file" << std:: endl;
				return 1;
			}

			if (optionFileListFields != 0) {
				switch (optionFileListFields) {
				case FLF_FilePath:
					return doPrintFileList(false, false);
				case FLF_FilePath | FLF_FileID:
					return doPrintFileList(true, false);
				case FLF_FilePath | FLF_Remark:
					return doPrintFileList(false, true);
				case FLF_FilePath | FLF_FileID | FLF_Remark:
					return doPrintFileList(true, true);
				case FLF_FileID:
				case FLF_FileID | FLF_Remark:
					assert(false); // internal error
					break;
				case FLF_Remark:
					return doPrintRemarks();
				case 0:
					assert(false);
					break;
				}
			}
			else if (optionCheckVersion) {
				boost::int32_t version[3];
				std:: string errorMessage;
				bool r = rawclonepair::read_version(inputFile, version, &errorMessage);
				if (r) {
					std:: cout << (boost::format("%d.%d.%d") % version[0] % version[1] % version[2]) << std:: endl;
					return 0;
				}
				else {
					std:: cerr << "error: " << errorMessage << std:: endl;
					return 1;
				}
			}
			else {
				return doPrettyPrint();
			}
		}
		std::cerr << "internal error" << std::endl;
		return 2; // error (internal)
	}
private:
	int doPrettyPrint()
	{
		if (ext == ".ccfxd" || ext == ".icetd") {
			std:: ofstream output;
			rawclonepair::RawClonePairPrinter printer;
			//printer.setAppVersionChecker(APPVERSION[0], APPVERSION[1]);
			printer.setOptionDefinitions(optionDefinitions);

			if (! outputFile.empty()) {
				output.open(outputFile.c_str(), std:: ios::out | std:: ios::binary);
				if (! output.is_open()) {
					std:: cerr << "error: can't create a file '" << outputFile << "'" << std:: endl;
					return 1;
				}
				printer.attachOutput(&output);
			}
			if (! printer.print(inputFile)) {
				std:: cerr << "error: " << printer.getErrorMessage() << std:: endl;
				return 1;
			}
		}
		else if (ext == ".ccfxprep") {
			std:: vector<std:: string> lines;
			if (! readPrepFile(&lines, inputFile)) {
				std:: cerr << "error: can't open a file '" << inputFile << "'" << std:: endl;
				return 1;
			}

			std:: ostream *pOutput = &std:: cout;
			std:: ofstream output;
			if (! outputFile.empty()) {
				output.open(outputFile.c_str(), std:: ios::out | std:: ios::binary);
				if (! output.is_open()) {
					std:: cerr << "error: can't create a file '" << outputFile << "'" << std:: endl;
					return 1;
				}
				pOutput = &output;
			}
			for (size_t i = 0; i < lines.size(); ++i) {
				(*pOutput) << lines[i] << std:: endl;
			}
		}

		return 0;
	}
	int doPrintFileList(bool withFileID, bool withRemark)
	{
		rawclonepair::RawClonePairFileAccessor accessor;
		std:: ostream *pOutput = &std:: cout;
		std:: ofstream output;
		if (! outputFile.empty()) {
			output.open(outputFile.c_str());
			if (! output.is_open()) {
				std:: cerr << "error: can't create a file '" << outputFile << "'" << std:: endl;
				return 1;
			}
			pOutput = &output;
		}
		if (! accessor.open(inputFile,
				rawclonepair::RawClonePairFileAccessor::FILEDATA
				| rawclonepair::RawClonePairFileAccessor::FILEREMARK)) {
			std:: cerr << "error: can't open a file '" << inputFile << "'" << std:: endl;
			return 1;
		}
		std::vector<int> fileIDs;
		accessor.getFiles(&fileIDs);
		for (size_t i = 0; i < fileIDs.size(); ++i) {
			int fileID = fileIDs[i];
			std:: string filePath;
			size_t fileSize;
			accessor.getFileDescription(fileID, &filePath, &fileSize);
			filePath = INNER2SYS(filePath);
			if (withFileID) {
				(*pOutput) << fileID << "\t";
			}
			(*pOutput).write(filePath.data(), filePath.length());
			(*pOutput) << std:: endl;

			if (withRemark) {
				std::vector<std::string> remark;
				accessor.getFileRemark(fileID, &remark);
				for (size_t j = 0; j < remark.size(); ++j) {
					std::string str = INNER2SYS(remark[j]);
					(*pOutput) << ":" << "\t";
					(*pOutput).write(str.data(), str.length());
					(*pOutput) << std::endl;
				}
			}
		}
		return 0;
	}
	int doPrintRemarks()
	{
		rawclonepair::RawClonePairFileAccessor accessor;
		std:: ostream *pOutput = &std:: cout;
		std:: ofstream output;
		if (! outputFile.empty()) {
			output.open(outputFile.c_str());
			if (! output.is_open()) {
				std:: cerr << "error: can't create a file '" << outputFile << "'" << std:: endl;
				return 1;
			}
			pOutput = &output;
		}
		if (! accessor.open(inputFile,
				rawclonepair::RawClonePairFileAccessor::FILEDATA
				| rawclonepair::RawClonePairFileAccessor::FILEREMARK)) {
			std:: cerr << "error: can't open a file '" << inputFile << "'" << std:: endl;
			return 1;
		}

		std::vector<int> fileIDs;
		accessor.getFileIDsHavingRemark(&fileIDs);
		for (size_t i = 0; i < fileIDs.size(); ++i) {
			int fileID = fileIDs[i];
			std::vector<std::string> remark;
			accessor.getFileRemark(fileID, &remark);
			if (! remark.empty()) {
				(*pOutput) << fileID << std::endl;
				for (size_t j = 0; j < remark.size(); ++j) {
					std::string str = INNER2SYS(remark[j]);
					(*pOutput) << ":" << "\t";
					(*pOutput).write(str.data(), str.length());
					(*pOutput) << std::endl;
				}
			}
		}
		return 0;
	}
private:
	static bool readPrepFile(std:: vector<std:: string> *pLines, const std:: string &fileName)
	{
		std:: vector<std:: string> &lines = *pLines;

		std:: ifstream is;
		is.open(fileName.c_str(), std:: ios::in | std::ios::binary);
		if (! is.good()) {
			return false;
		}
		
		std:: string str;
		while (true) {
			str.clear();
			char ch;
			while (is.get(ch)) {
				if (ch == '\n' || ch == '\r' || ch == EOF) {
					break; // while
				}
				str += ch;
			}
			if (ch != '\n') {
				break; // while
			}
			
			if (str.length() >= 1 && str[str.length() - 1] == '\r') {
				str.resize(str.length() - 1);
			}
			if (str.empty()) {
				if (! is.eof()) {
					return false;
				}
				break; // while
			}
			lines.push_back(str);
		}
		
		return true; // success
	}
};

#endif //defined PRETTYPRINTMAIN_H
