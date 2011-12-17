#if ! defined CLONEDATAASSEMBLER_H
#define CLONEDATAASSEMBLER_H

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "../common/hash_map_includer.h"
#include <exception>

#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../common/ffuncrenamer.h"

#include "../common/utf8support.h"
#include "../common/unportable.h"
#include "ccfxcommon.h"
#include "rawclonepairdata.h"

class CloneDataAssembler {
private:
	struct closeAndDelete {
	private:
		FILE *p;
		std::string filePath;
	public:
		closeAndDelete(FILE *p_, const std::string &filePath_)
			: p(p_), filePath(filePath_)
		{
		}
		~closeAndDelete() 
		{
			if (p != NULL) {
				fclose(p);
			}
			if (! filePath.empty()) {
				::remove(filePath.c_str());
			}
		}
		void reset()
		{
			p = NULL;
			filePath.clear();
		}
	};
private:
	Decoder defaultDecoder;
	std:: string SYS2INNER(const std::string &systemString)
	{
		return toUTF8String(defaultDecoder.decode(systemString));
	}
	std:: string INNER2SYS(const std::string &innerString)
	{
		return defaultDecoder.encode(toWStringV(innerString));
	}
public:
	class Error : public std::runtime_error {
	public:
		unsigned long long lineNumber;
	public:
		Error(const std::string &errorMessage_, unsigned long long lineNumber_)
			: std::runtime_error(errorMessage_), lineNumber(lineNumber_)
		{
		}
	};
private:
	bool getline(std::ifstream *pInput, std::string *pBuffer, long long *pLineNumber)
	{
		if ((*pInput).eof()) {
			return false;
		}

		std::string &buffer = *pBuffer;

		std::getline(*pInput, buffer, '\n');
		int ch = buffer[buffer.length() - 1];
		if (buffer.length() > 0 && buffer[buffer.length() - 1] == '\r') {
			buffer.resize(buffer.length() - 1);
		}
		++*pLineNumber;
		return true;
	}
public:
	void assemble(const std::string &outputFile, const std::string &inputFile) // throw CloneDataAssember::Error
	{
		std:: ifstream input;
		input.open(inputFile.c_str(), std:: ios::in | std:: ios::binary);
		if (! input.is_open()) {
			throw Error("can't open the input file", 0);
		}

		std::string tempFilePath = make_temp_file_on_the_same_directory(outputFile, "ccfxd", ".tmp");
		FILE *poutp = fopen(tempFilePath.c_str(), "wb");

		if (poutp == NULL) {
			throw Error("can't create the output file", 0);
		}
		closeAndDelete candd(poutp, tempFilePath);

		static const char signature[] = "ccfxraw0";
		FWRITEBYTES(signature, 8, poutp);

		std::string str;
		long long lineNumber = 0;
		getline(&input, &str, &lineNumber);
		if (str.empty() || str.length() >= 80) {
			throw Error("invalid header", lineNumber);
		}
		int v1, v2, v3;
		{
			if (sscanf(str.c_str(), "version: ccfx %d.%d.%d", &v1, &v2, &v3) != 3) {
				throw Error("invalid header", lineNumber);
			}
			boost::int32_t d[3] = { v1, v2, v3 };
			for (int i = 0; i < 3; ++i) {
				flip_endian(&d[i], sizeof(boost::int32_t));
				FWRITE(&d[i], sizeof(boost::int32_t), 1, poutp);
			}
		}
		if (v1 != 10) {
			throw Error("version mismatch", lineNumber);
		}
		switch (v2) {
		case 1:
		case 2:
			// ok
			break;
		default:
			throw Error("version mismatch", lineNumber);
			break;
		}

		getline(&input, &str, &lineNumber);
		if (str != "format: pair_diploid") {
			throw Error("invalid format descriptor", lineNumber);
		}
		FWRITEBYTES("pa:d", 4, poutp);

		if (v2 == 1) {		
			static const char options[] = { 'b', 's', 'u', 't', '\0' };
			std::vector<boost::int32_t> values;
			for (int i = 0; true; ++i) {
				getline(&input, &str, &lineNumber);
				if (options[i] == '\0') {
					break; // for i
				}
				if (str.empty() || str.length() >= 80) {
					break; // for i
				}
				std::string optionColon_ = "option: ";
				if (! boost::algorithm::starts_with(str, optionColon_)) {
					break; // for i
				}
				std::string s2 = str.substr(optionColon_.length());
				if (! (s2.length() >= 3 && s2[0] == '-' && s2[1] == options[i] && s2[2] == ' ')) {
					throw Error("invalid option name", lineNumber);
				}
				std::string name = s2.substr(1, 1);
				if (! is_name(name)) {
					throw Error("invalid option name", lineNumber);
				}
				std::string s3 = s2.substr(3);
				int value;
				if (sscanf(s3.c_str(), "%d", &value) != 1 || value < 0) { // the number may be followd by comments, like "option: -b 50 // minimumCloneLength".
					throw Error("invalid option value", lineNumber);
				}

				values.push_back(value);
			}
			boost::uint32_t countOfOptions = values.size();
			assert(0 <= countOfOptions && countOfOptions <= 4);
			flip_endian(&countOfOptions, sizeof(boost::uint32_t));
			FWRITE(&countOfOptions, sizeof(boost::uint32_t), 1, poutp);
			for (size_t i = 0; i < countOfOptions; ++i) {
				flip_endian(&values[i], sizeof(boost::uint32_t));
				FWRITE(&values[i], sizeof(boost::uint32_t), 1, poutp);
			}
		}
		else if (v2 == 2) {
			for (int i = 0; true; ++i) {
				getline(&input, &str, &lineNumber);
				if (str.empty() || str.length() >= 80) {
					break; // for i
				}
				std::string optionColon_ = "option: ";
				if (! boost::algorithm::starts_with(str, optionColon_)) {
					break; // for i
				}
				std::string s2 = str.substr(optionColon_.length());
				if (! (s2.length() >= 2 && s2[0] == '-')) {
					throw Error("invalid option", lineNumber);
				}
				std::string s3 = s2.substr(1);
				std::string::size_type p = s3.find(' ');
				if (p == std::string::npos) {
					throw Error("invalid option", lineNumber);
				}
				std::string name = s3.substr(0, p);
				std::string value = s3.substr(p + 1);
				if (! is_name(name)) {
					throw Error("invalid option name", lineNumber);
				}
				std::string valueUtf8 = this->SYS2INNER(value);
				if (! is_utf8_nocontrol(valueUtf8)) {
					throw Error("invalid option value", lineNumber);
				}
				FWRITEBYTES(name.data(), name.length(), poutp);
				FPUTC('\t', poutp);
				FWRITEBYTES(value.data(), value.length(), poutp);
				FPUTC('\n', poutp);
			}
			FPUTC('\n', poutp);
		}
		else {
			assert(false);
		}

		// here, str has already value
		{
			const std::string preprocessScriptColon_ = "preprocess_script: ";
			if (! boost::algorithm::starts_with(str, preprocessScriptColon_)) {
				throw Error("invalid preprocess script", lineNumber);
			}
			std::string value = str.substr(preprocessScriptColon_.length());
			FWRITEBYTES(value.data(), value.length(), poutp);
			FPUTC('\n', poutp);
		}

		HASH_MAP<boost::int32_t, boost::int32_t> fileIDToLength;
		getline(&input, &str, &lineNumber);
		if (str != "source_files {") {
			throw Error("invalid source-file section", lineNumber);
		}
		{
			while (true) {
				getline(&input, &str, &lineNumber);
				if (str.empty()) {
					throw Error("invalid file description", lineNumber);
				}
				if (str == "}") {
					break; // while
				}
				std::vector<std::string> fields;
				boost::split(fields, str, boost::is_any_of("\t"));
				if (fields.size() != 3) {
					throw Error("invalid file description", lineNumber);
				}
				const std::string &idStr = fields[0];
				const std::string &pathStr = fields[1];
				const std::string &lenStr = fields[2];

				boost::int32_t id = 0;
				try {
					id = boost::lexical_cast<boost::int32_t, std::string>(idStr);
				}
				catch (boost::bad_lexical_cast &) {
					throw Error("invalid file ID", lineNumber);
				}
				if (fileIDToLength.find(id) != fileIDToLength.end()) {
					throw Error("file ID conflict", lineNumber);
				}
				boost::int32_t len = 0;
				try {
					len = boost::lexical_cast<boost::int32_t, std::string>(lenStr);
				}
				catch (boost::bad_lexical_cast &) {
					throw Error("invalid file length", lineNumber);
				}

				fileIDToLength[id] = len;

				std::string pathStrUtf8 = SYS2INNER(pathStr);
				flip_endian(&id, sizeof(boost::int32_t));
				flip_endian(&len, sizeof(boost::int32_t));
				FWRITEBYTES(pathStrUtf8.data(), pathStrUtf8.length(), poutp);
				FWRITE("\n", sizeof(char), 1, poutp);
				FWRITE(&id, sizeof(boost::int32_t), 1, poutp);
				FWRITE(&len, sizeof(boost::int32_t), 1, poutp);
			}
			if (v2 == 1) {
				fwrite("\n", sizeof(char), 1, poutp);
			}
			else if (v2 == 2) {
				boost::int32_t id = 0;
				boost::int32_t len = 0;
				FPUTC('\n', poutp);
				FWRITE(&id, sizeof(boost::int32_t), 1, poutp);
				FWRITE(&len, sizeof(boost::int32_t), 1, poutp);
			}
		}

		if (v2 == 2) {
			getline(&input, &str, &lineNumber);
			if (str != "source_file_remarks {") {
				throw Error("missing source-file remark section", lineNumber);
			}
			boost::optional<boost::int32_t> fileID;
			while (true) {
				getline(&input, &str, &lineNumber);
				if (str.empty()) {
					throw Error("invalid file remark", lineNumber);
				}
				if (str == "}") {
					break; // while
				}
				std::string strUtf8 = SYS2INNER(str);
				if (strUtf8.length() >= 2 && strUtf8[0] == ':' && strUtf8[1] == '\t') {
					if (! fileID) {
						throw Error("file remark text appears before file ID", lineNumber);
					}

					std::string commentStrUtf8 = strUtf8.substr(2);
					if (! is_utf8_nocontrol(commentStrUtf8)) {
						throw Error("invalid file remark text", lineNumber);
					}
					
					boost::int32_t id = *fileID;
					flip_endian(&id, sizeof(boost::int32_t));
					FWRITEBYTES(commentStrUtf8.data(), commentStrUtf8.length(), poutp);
					FPUTC('\n', poutp);
					FWRITE(&id, sizeof(boost::int32_t), 1, poutp);
				}
				else {
					boost::int32_t id = 0;
					try {
						id = boost::lexical_cast<boost::int32_t, std::string>(strUtf8);
					}
					catch (boost::bad_lexical_cast &) {
						throw Error("invalid file ID", lineNumber);
					}
					fileID = id;
				}
			}
			{
				boost::int32_t id = 0;
				FPUTC('\n', poutp);
				FWRITE(&id, sizeof(boost::int32_t), 1, poutp);
			}
		}

		getline(&input, &str, &lineNumber);
		if (str != "clone_pairs {") {
			throw Error("invalid clone-pair section", lineNumber);
		}
		{
			while (true) {
				getline(&input, &str, &lineNumber);
				if (str.empty()) {
					throw Error("invalid clone-pair description", lineNumber);
				}
				if (str == "}") {
					break; // while
				}
				std::vector<std::string> fields;
				boost::split(fields, str, boost::is_any_of("\t"));
				if (fields.size() != 3) {
					throw Error("invalid clone-pair description", lineNumber);
				}
				boost::uint64_t cloneIDValue;
				try {
					cloneIDValue = boost::lexical_cast<boost::uint64_t, std::string>(fields[0]);
				}
				catch (boost::bad_lexical_cast &) {
					throw Error("invalid clone-pair description", lineNumber);
				}

				boost::array<int, 3> lefts;
				if (sscanf(fields[1].c_str(), "%d.%d-%d", &lefts[0], &lefts[1], &lefts[2]) != 3) {
					throw Error("invalid clone-pair description", lineNumber);
				}
				boost::array<int, 3> rights;
				if (sscanf(fields[2].c_str(), "%d.%d-%d", &rights[0], &rights[1], &rights[2]) != 3) {
					throw Error("invalid clone-pair description", lineNumber);
				}
				rawclonepair::RawClonePair clonePair(
					rawclonepair::RawFileBeginEnd(lefts[0], lefts[1], lefts[2]), 
					rawclonepair::RawFileBeginEnd(rights[0], rights[1], rights[2]),
					cloneIDValue);

				{
					HASH_MAP<boost::int32_t, boost::int32_t>::const_iterator i = fileIDToLength.find(clonePair.left.file);
					if (i == fileIDToLength.end()) {
						throw Error("unknown file ID", lineNumber);
					}
					boost::int32_t leftLen = i->second;
					if (! (0 <= clonePair.left.begin && clonePair.left.begin <= clonePair.left.end && clonePair.left.end <= leftLen)) {
						throw Error("wrong potision", lineNumber);
					}
				}
				{
					HASH_MAP<boost::int32_t, boost::int32_t>::const_iterator i = fileIDToLength.find(clonePair.right.file);
					if (i == fileIDToLength.end()) {
						throw Error("unknown file ID", lineNumber);
					}
					boost::int32_t rightLen = i->second;
					if (! (0u < clonePair.right.begin && clonePair.right.begin <= clonePair.right.end && clonePair.right.end <= rightLen)) {
						throw Error("wrong potision", lineNumber);
					}
				}

				fwrite_RawClonePair(&clonePair, 1, poutp);
			}
			rawclonepair::RawClonePair nullClonePair;
			fwrite_RawClonePair(&nullClonePair, 1, poutp);
		}

		if (v2 == 2) {
			getline(&input, &str, &lineNumber);
			if (str != "clone_set_remarks {") {
				throw Error("missing clone-set remark section", lineNumber);
			}
			boost::optional<boost::uint64_t> cloneSetID;
			while (true) {
				getline(&input, &str, &lineNumber);
				if (str.empty()) {
					throw Error("invalid clone-set remark", lineNumber);
				}
				if (str == "}") {
					break; // while
				}
				std::string strUtf8 = SYS2INNER(str);
				if (strUtf8.length() >= 2 && strUtf8[0] == ':' && strUtf8[1] == '\t') {
					if (! cloneSetID) {
						throw Error("clone-set remark text appears before clone-set ID", lineNumber);
					}

					std::string commentStrUtf8 = strUtf8.substr(2);
					if (! is_utf8_nocontrol(commentStrUtf8)) {
						throw Error("invalid clone-set remark text", lineNumber);
					}
					
					boost::uint64_t id = *cloneSetID;
					flip_endian(&id, sizeof(boost::uint64_t));
					FWRITEBYTES(commentStrUtf8.data(), commentStrUtf8.length(), poutp);
					FPUTC('\n', poutp);
					FWRITE(&id, sizeof(boost::uint64_t), 1, poutp);
				}
				else {
					boost::uint64_t id = 0;
					try {
						id = boost::lexical_cast<boost::uint64_t, std::string>(strUtf8);
					}
					catch (boost::bad_lexical_cast &) {
						throw Error("invalid clone-set ID", lineNumber);
					}
					cloneSetID = id;
				}
			}
			{
				boost::uint64_t id = 0;
				FPUTC('\n', poutp);
				FWRITE(&id, sizeof(boost::uint64_t), 1, poutp);
			}
		}


		candd.reset();
		::remove(outputFile.c_str());
		fclose(poutp);
		if (::rename(tempFilePath.c_str(), outputFile.c_str()) != 0) {
			throw Error("can't create the output file", 0);
		}
	}
};

#endif // CLONEDATAASSEMBLER_H
