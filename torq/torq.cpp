#include <cstdio>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include "../common/hash_map_includer.h"
#include <algorithm>
#include <stdexcept>

#include <boost/format.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/scoped_ptr.hpp>

#include "../common/unportable.h" // import make_temp_file_on_the_same_directory
#include "../common/utf8support.h"
#ifdef _MSC_VER
#include "../common/win32streamhandle.h"
#endif
#include "torqcommon.h"
#include "torqtokenizer.h"
#include "torqparser.h"
#include "texttoken.h"
#include "interpreter.h"

#if defined BUILD_TORQDLL
#elif defined BUILD_TORQEXE
#else
#error "specify build target"
#endif

#if defined BUILD_TORQ_MULTITHREADEXE
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#endif

const common::Version appVersion(0, 4, 3);
const common::Version dataVersion(0, 4, 3);

#if defined _MSC_VER

HANDLE openCompressedFileForWrite(const std::string &fileName)
{
	{
		HANDLE hFile = CreateFile(
			fileName.c_str(),
			GENERIC_WRITE | GENERIC_WRITE,
			0,
			0,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			0);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		CloseHandle(hFile);
	}
	{
		HANDLE hFile = CreateFile (
			fileName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, 
			0,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			0);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		USHORT compFormat = COMPRESSION_FORMAT_DEFAULT;
		DWORD dw;
		BOOL success = DeviceIoControl(
			(HANDLE)hFile,            // handle to file or directory
			FSCTL_SET_COMPRESSION,       // dwIoControlCode
			(LPVOID) &compFormat,         // input buffer
			(DWORD) sizeof(USHORT),       // size of input buffer
			NULL,                        // lpOutBuffer
			0,                           // nOutBufferSize
			(LPDWORD) &dw,   // number of bytes returned
			(LPOVERLAPPED)NULL  // OVERLAPPED structure
			);
#if defined HAVE_TO_BE_COMPRESSED
		if (! success) {
			CloseHandle(hFile);
			return false;
		}
#endif
		return hFile;
	}
}

#endif

bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines)
{
	(*pLines).clear();

	std:: ifstream input;
	input.open(file_path.c_str(), std:: ios::in | std:: ios::binary);
	if (! input.is_open()) {
		return false;
	}
	
	input.seekg(0, std:: ios::end);
	size_t inputSize = input.tellg();
	input.seekg(0, std:: ios::beg);
	
	std:: vector<char> buf;
	buf.resize(inputSize);
	input.read(&buf[0], inputSize);
	input.close();
	
	size_t i = 0;
	
	// remove BOM
	if (buf.size() >= 3 
			&& ((unsigned char)buf[0]) == 0xef && ((unsigned char)buf[1]) == 0xbb && ((unsigned char)buf[2]) == 0xbf) {
		i += 3;
	}
	
	while (i < buf.size()) {
		size_t p = i;
		int ch;
		while (p < buf.size() && (ch = buf[i]) != '\r' && ch != '\n') {
			++p;
		}
		size_t len = p - i;
		std:: string line(&buf[i], len);
		(*pLines).push_back(line);
		if (p < buf.size()) {
			ch = buf[p];
			if (ch == '\r') {
				if (p + 1 < buf.size() && buf[p + 1] == '\n') {
					i = p + 2;
				}
				else {
					i = p + 1;
				}
			}
			else if (ch == '\n') {
				i = p + 1;
			}
			else {
				assert(false);
			}
		}
	}

	return true; 
}

std:: pair<boost::int32_t/* row */, boost::int32_t /* col */> posToRowCol(const std:: vector<MYWCHAR_T> &script, boost::int32_t pos)
{
	boost::int32_t lineNumber = 1;
	boost::int32_t columnNumber = 1;
	boost::int32_t p = 0; 
	while (p < pos) {
		if (script[p] == '\r' || script[p] == '\n') {
			++lineNumber;
			columnNumber = 1;
			if (script[p] == '\r' && p + 1 < pos && script[p + 1] == '\n') {
				p += 2;
			}
			else {
				++p;
			}
		}
		else {
			++columnNumber;
			++p;
		}
	}
	return std:: pair<boost::int32_t, boost::int32_t>(lineNumber, columnNumber);
}

bool read(std:: vector<MYWCHAR_T> *pSeq, const std:: string &scriptFile)
{
	std:: vector<MYWCHAR_T> &seq = *pSeq;
	seq.resize(0);
	std:: ifstream input;
	input.open(scriptFile.c_str(), std:: ios::in | std:: ios::binary);
	if (! input.is_open()) {
		return false;
	}
	
	input.seekg(0, std:: ios::end);
	size_t inputSize = input.tellg();
	input.seekg(0, std:: ios::beg);
	
	std:: vector<char> buf;
	buf.resize(inputSize);
	input.read(&buf[0], inputSize);
	input.close();

	size_t i = 0;
	
	// remove BOM
	if (buf.size() >= 3 
			&& ((unsigned char)buf[0]) == 0xef && ((unsigned char)buf[1]) == 0xbb && ((unsigned char)buf[2]) == 0xbf) {
		i += 3;
	}
	
	toWStringV(&seq, &buf[i], buf.size() - i);

	return true;
}

class FileReader
{
public:
	virtual ~FileReader()
	{
	}
	virtual bool read(std:: vector<MYWCHAR_T> *pSeq, const std:: string &fileName) = 0;
	virtual std:: string getErrorMessage() const = 0;
	virtual FileReader *dup() const = 0;
};

class EncodedFileReader : public FileReader
{
private:
	std:: string errorMessage;
	Decoder decoder;
public:
	FileReader *dup() const
	{
		return new EncodedFileReader(*this);
	}
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

bool check_special_chars(const std:: vector<MYWCHAR_T> &str)
{
	size_t i = 0;
	while (i < str.size()) {
		MYWCHAR_T ch = str[i];
		if (ch == '%') {
			if (! (i + 1 < str.size())) {
				return false;
			}
			ch = str[i + 1];
			switch (ch) {
			case '%':
			case 'c':
			case 'i':
			case 'n':
			case 'N':
			case 'r':
			case 'S':
			case 't':
			case 'T':
				break;
			default:
				return false;
			}
			i += 2;
		}
		else {
			++i;
		}
	}
	return true;

}

bool scan_option_node_format(const std:: vector<MYWCHAR_T> &f, size_t beginPos, size_t endPos,
							 std:: vector<MYWCHAR_T> *pLabel, text::Helper::NodeFormat *pOpenClose)
{
	text::Helper::NodeFormat openClose;
	
	const std:: vector<MYWCHAR_T>::const_iterator iBegin = f.begin() + beginPos;
	const std:: vector<MYWCHAR_T>::const_iterator iEnd = f.begin() + endPos;

	std:: vector<MYWCHAR_T>::const_iterator i = iBegin;
	MYWCHAR_T ch;
	while (i != iEnd && ((ch = *i) == '_' || 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9')) {
		++i;
	}
	if (i == iEnd) {
		return false;
	}
	ch = *i;
	(*pLabel).clear();
	(*pLabel).insert((*pLabel).end(), iBegin, i);
	if (ch == ':') {
		std:: vector<MYWCHAR_T>::const_iterator coloni = i;
		std:: vector<MYWCHAR_T> v;
		v.insert(v.end(), coloni + 1, iEnd);
		std:: string s = common::EscapeSequenceHelper::encode(v, false);
		if (s == "flatten") {
			// label:flatten
			openClose.nodeType = text::Helper::NF_EXPANDED;
			(*pOpenClose).swap(openClose);
			return true;
		}
		else if (s == "none") {
			// label:none
			openClose.nodeType = text::Helper::NF_NONE;
			(*pOpenClose).swap(openClose);
			return true;
		}
		else if (s == "terminate") {
			// label:terminate
			openClose.nodeType = text::Helper::NF_TERMINATED;
			openClose.opening.insert(openClose.opening.end(), iBegin, coloni);
			(*pOpenClose).swap(openClose);
			return true;
		}
		else {
			return false;
		}
	}
	else if (ch == '=') {
		// label=replacer
		if (i + 1 == iEnd) {
			openClose.nodeType = text::Helper::NF_NONE;
			(*pOpenClose).swap(openClose);
		}
		else {
			openClose.nodeType = text::Helper::NF_TERMINATED;
			openClose.opening.insert(openClose.opening.end(), i + 1, iEnd);
			(*pOpenClose).swap(openClose);
		}
		return true;
	}
	else {
		std:: vector<MYWCHAR_T>::const_iterator comma1i = i;
		i = comma1i + 1;
		std:: vector<MYWCHAR_T>::const_iterator comma2i = std:: find(i, iEnd, ch);
		if (comma2i != iEnd) {
			// label,open,close
			i = comma2i + 1;
			openClose.nodeType = text::Helper::NF_EXPANDED;
			openClose.opening.insert(openClose.opening.end(), comma1i + 1, comma2i);
			if (! check_special_chars(openClose.opening)) {
				return false;
			}
			openClose.closing.insert(openClose.closing.end(), comma2i + 1, iEnd);
			(*pOpenClose).swap(openClose);
			return true;
		}
		else {
			return false;
		}
	}
}

bool scan_option_node_format(const std:: string &f, std:: vector<MYWCHAR_T> *pLabel, text::Helper::NodeFormat *pOpenClose)
{
	std:: vector<MYWCHAR_T> line; 
	common::EscapeSequenceHelper::decode(&line, f);
	return scan_option_node_format(line, 0, line.size(), pLabel, pOpenClose);
}

void print_result_cng(std:: ostream *pOutput, 
		const std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormats0,
		const std:: vector<std:: vector<MYWCHAR_T> > &labelStrings,
		const text::TokenSequence &text)
{
	HASH_MAP<boost::int32_t/* code */, text::Helper::NodeFormat> nodeFormats;
	{
		std:: vector<MYWCHAR_T> closingLabel;
		for (size_t i = 0; i < labelStrings.size(); ++i) {
			const std:: vector<MYWCHAR_T> &label = labelStrings[i];
			assert(label.size() > 0);
			std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats0.find(label);
			if (i0 == nodeFormats0.end()) {
				nodeFormats[(boost::int32_t)i] = text::Helper::NodeFormat(text::Helper::NF_TERMINATED, label, closingLabel); // the default is "terminated"
			}
			else {
				nodeFormats[(boost::int32_t)i] = i0->second;
			}
		}
	}

	text::Helper::printCng(pOutput, text, nodeFormats);
}

void print_result_xml(std:: ostream *pOutput, 
		unsigned long options,
		const std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormats0,
		const std:: vector<std:: vector<MYWCHAR_T> > &labelStrings,
		const text::TokenSequence &text)
{
	std:: vector<MYWCHAR_T> strRaw;
	common::EscapeSequenceHelper::decode(&strRaw, "raw");
	text::Helper::printXml(pOutput, text, strRaw, options, labelStrings);
}

std:: vector<MYWCHAR_T> replace(const std:: vector<MYWCHAR_T> &str, 
		const std:: vector<MYWCHAR_T> &from, const std:: vector<MYWCHAR_T> &to)
{
	assert(from.size() > 0);
	std:: vector<MYWCHAR_T> r;
	size_t i = 0;
	while (i < str.size()) {
		MYWCHAR_T ch = str[i];
		if (i + from.size() <= str.size()) {
			bool match = true;
			for (size_t j = 0; j < from.size(); ++j) {
				if (str[i + j] != from[j]) {
					match = false;
					break; // for j
				}
			}
			if (match) {
				r.insert(r.end(), to.begin(), to.end());
				i += from.size();
			}
			else {
				r.push_back(str[i]);
				++i;
			}
		}
		else {
			r.push_back(str[i]);
			++i;
		}
	}
	return r;
}

std:: vector<MYWCHAR_T> staticExpandSpecials(const std:: vector<MYWCHAR_T> &str)
{
	std:: vector<MYWCHAR_T> r;
	size_t i = 0;
	while (i < str.size()) {
		MYWCHAR_T ch = str[i];
		if (ch == '%') {
			if (i + 1 < str.size()) {
				ch = str[i + 1];
				switch (ch) {
				case 'S':
					r.push_back(' ');
					i += 2;
					break;
				case 'T':
					r.push_back('\t');
					i += 2;
					break;
				case 'N':
					r.push_back('\n');
					i += 2;
					break;
				default:
					r.push_back('%');
					++i;
				}
			}
			else {
				r.push_back(ch);
				++i;
			}
		}
		else {
			r.push_back(ch);
			++i;
		}
	}
	return r;
}

void print_result(std:: ostream *pOutput, 
		unsigned long options,
		const std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormats0,
		const std:: vector<std:: vector<MYWCHAR_T> > &labelStrings,
		const common::Encoder *pRawCharEncoder, const common::Encoder *pGeneratedEncoder,
		const text::TokenSequence &text)
{
	HASH_MAP<boost::int32_t/* code */, text::Helper::NodeFormat> nodeFormats;
	assert(labelStrings.size() >= 1);
	
	std:: vector<MYWCHAR_T> nullLabel;
	std:: vector<MYWCHAR_T> percentT;
	common::EscapeSequenceHelper::decode(&percentT, "%t");
	std:: vector<MYWCHAR_T> percentLargeN;
	common::EscapeSequenceHelper::decode(&percentLargeN, "%N");
	std:: vector<MYWCHAR_T> newLine;
	newLine.push_back('\n');
	std:: vector<MYWCHAR_T> percentLargeT;
	common::EscapeSequenceHelper::decode(&percentLargeT, "%T");
	std:: vector<MYWCHAR_T> tabChar;
	tabChar.push_back('\t');

	text::Helper::NodeFormat rawTokenFormat;
	{
		std:: vector<MYWCHAR_T> label;
		common::EscapeSequenceHelper::decode(&label, "_tok");
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats0.find(label);
		if (i0 != nodeFormats0.end()) {
			rawTokenFormat = i0->second;
			rawTokenFormat.opening = staticExpandSpecials(replace(rawTokenFormat.opening, label, percentT));
			rawTokenFormat.closing = staticExpandSpecials(replace(rawTokenFormat.closing, label, percentT));
		}
		else {
			rawTokenFormat.nodeType = text::Helper::NF_EXPANDED;
			common::EscapeSequenceHelper::decode(&rawTokenFormat.opening, "%t[");
			rawTokenFormat.closing.push_back(']');
		}
	}

	for (size_t i = 0; i < labelStrings.size(); ++i) {
		std:: vector<MYWCHAR_T> label = labelStrings[i];
		assert(label.size() > 0);
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats0.find(label);
		if (i0 != nodeFormats0.end()) {
			text::Helper::NodeFormat &ni = nodeFormats[(boost::int32_t)i];
			ni = i0->second;
			ni.opening = staticExpandSpecials(ni.opening);
			ni.closing = staticExpandSpecials(ni.closing);
		}
		else if (i == 0 /* null */ && (options & text::Helper::SkipNull) != 0) {
			NULL;
		}
		else if (i == 0 /* null */ && (options & text::Helper::RecurseNull) == 0) {
			text::Helper::NodeFormat &ni = nodeFormats[(boost::int32_t)i];
			ni = text::Helper::NodeFormat(text::Helper::NF_TERMINATED, label, nullLabel);
			ni.opening = staticExpandSpecials(ni.opening);
			ni.closing = staticExpandSpecials(ni.closing);
		}
		else {
			std:: vector<MYWCHAR_T> s = staticExpandSpecials(replace(rawTokenFormat.opening, percentT, label));
			std:: vector<MYWCHAR_T> t = staticExpandSpecials(replace(rawTokenFormat.closing, percentT, label));
			nodeFormats[(boost::int32_t)i] = text::Helper::NodeFormat(rawTokenFormat.nodeType, s, t);
		}
	}

	text::Helper::NodeFormat rawTextFormat;
	{
		std:: vector<MYWCHAR_T> label;
		common::EscapeSequenceHelper::decode(&label, "_raw");
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats0.find(label);
		if (i0 != nodeFormats0.end()) {
			rawTextFormat = i0->second;
		}
		else {
			rawTextFormat.nodeType = text::Helper::NF_EXPANDED;
			rawTextFormat.opening.push_back('\"');
			rawTextFormat.closing.push_back('\"');
		}
	}

	std:: vector<MYWCHAR_T> separator;
	{
		std:: vector<MYWCHAR_T> label;
		common::EscapeSequenceHelper::decode(&label, "_sep");
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats0.find(label);
		if (i0 != nodeFormats0.end()) {
			separator = i0->second.opening;
		}
		else {
			separator.push_back(' ');
		}
		separator = staticExpandSpecials(separator);
	}
	{
		text::Helper::print(pOutput, text, rawTextFormat, separator, options, nodeFormats, pRawCharEncoder, pGeneratedEncoder);
		(*pOutput) << std:: endl;
	}
}

int debug_print_trace(const std:: vector<TRACE_ITEM> &trace0, const TorqTokenizer &tokenizer)
{
	std:: vector<TRACE_ITEM> trace = Interpreter::remove_unrequired_items(trace0);
	TraceDirector td;
	td.setTrace(trace);
	for (size_t i = 0; i < trace.size(); ++i) {
		const TRACE_ITEM &it = trace[i];
		std:: string dinfo = (boost::format("[%d %d %d]") % td.findPair(i) % td.findParent(i) % td.findChildren(i).size()).str();
		if (it.classification == TRACE_ITEM::Enter) {
			std:: cout << i << ": < " << it.begin << " " << (NodeClassificationHelper::toString(it.node)) << " " << dinfo << std:: endl;
		}
		else if (it.classification == TRACE_ITEM::Reject) {
			std:: cout << i << ": >Reject " << it.begin << " " << (NodeClassificationHelper::toString(it.node)) << " " << dinfo << std:: endl;
		}
		else {
			const TOKEN &ti = it.ref;
			if (ti.classification != TOKEN::NUL) {
				std:: vector<MYWCHAR_T> str = tokenizer.getTokenString(ti);
				std:: string s = toUTF8String(&str[0], str.size());
				std:: cout << i << ": > " << it.begin << " " << (NodeClassificationHelper::toString(it.node)) << "(" << s << ")" << " " << dinfo << std:: endl;
			}
			else {
				std:: cout << i << ": > " << it.begin << " " << (NodeClassificationHelper::toString(it.node)) << " " << dinfo << std:: endl;
			}
		}
	}
	return 0;
}

void extract_option_lines(const std:: vector<MYWCHAR_T> &script, std:: map<boost::int32_t /* line number */, std:: vector<MYWCHAR_T> > *pOptionLines)
{
	std:: map<boost::int32_t /* line number */, std:: vector<MYWCHAR_T> > optionLines;
	boost::int32_t lineNumber = 1;
	size_t lineBeginPos = 0;
	while (lineBeginPos < script.size()) {
		bool isOptionLine = script[lineBeginPos] == '-';
		size_t lineEndPos = lineBeginPos;
		while (lineEndPos < script.size() && script[lineEndPos] != '\n' && script[lineEndPos] != '\r') {
			++lineEndPos;
		}
		if (isOptionLine) {
			std:: vector<MYWCHAR_T> &line = optionLines[lineNumber];
			line.insert(line.end(), script.begin() + lineBeginPos, script.begin() + lineEndPos);
		}
		lineBeginPos = lineEndPos;
		if (lineBeginPos + 1 < script.size() && script[lineBeginPos] == '\r' && script[lineBeginPos + 1] == '\n') {
			lineBeginPos += 2; // skip '\r' '\n'
		}
		else {
			++lineBeginPos; // skip '\n', or '\r'
		}
		++lineNumber;
	}
	(*pOptionLines).swap(optionLines);
}

bool parse_script(const std:: vector<MYWCHAR_T> &script, std:: vector<TRACE_ITEM> *pTrace, 
	std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> *pNodeFormatsInScript, 
	std:: string *pErrorMessage, const std:: string &debugOption)
{
	assert(pTrace != NULL);
	assert(pNodeFormatsInScript != NULL);
	assert(pErrorMessage != NULL);

	std:: vector<TRACE_ITEM> &trace = *pTrace;
	std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormatsInScript = *pNodeFormatsInScript;

	// extract format options
	std:: map<boost::int32_t /* line number */, std:: vector<MYWCHAR_T> > optionLines;
	extract_option_lines(script, &optionLines);
	for (std:: map<boost::int32_t /* line number */, std:: vector<MYWCHAR_T> >::const_iterator i = optionLines.begin(); i != optionLines.end(); ++i) {
		static const std:: string strNodeformat = "--nodeformat";
		const std:: vector<MYWCHAR_T> &line = i->second;
		MYWCHAR_T c;
		if (common::substrEqual(line, 0, strNodeformat) && strNodeformat.length() < line.size() 
				&& ((c = line[strNodeformat.length()]) == ' ' || c == '\t')) {
			size_t pos = strNodeformat.length();
			while (pos < line.size() && ((c = line[pos]) == ' ' || c == '\t')) {
				++pos;
			}
			std:: vector<MYWCHAR_T> label;
			text::Helper::NodeFormat openClose;
			size_t endPos = std:: find(line.begin(), line.end(), '\n') - line.begin();
			if (! scan_option_node_format(line, pos, endPos, &label, &openClose)) {
				*pErrorMessage = (boost::format("line %d: invalid --nodeformat argument") % (int)i->first).str();
				return false;
			}
			nodeFormatsInScript[label].swap(openClose);
		}
		else {
			*pErrorMessage = (boost::format("line %d: syntax error") % (int)i->first).str();
			return false;
		}
	}

	//std:: cerr << "includes " << script.size() << " chars" << std:: endl;

	// tokenize the script
	TorqTokenizer tokenizer;
	tokenizer.setScript(script);
	if (! tokenizer.tokenize()) {
		boost::int32_t errorPos = tokenizer.getErrorPos();
		std:: pair<boost::int32_t, boost::int32_t> rowCol = posToRowCol(script, errorPos);
		size_t p = errorPos + 20;
		if (p > script.size()) {
			p = script.size();
		}
		std:: vector<MYWCHAR_T> subscr;
		subscr.insert(subscr.end(), script.begin() + errorPos, script.begin() + p);
		std:: string subscrstr = toUTF8String(&subscr[0], subscr.size());
		*pErrorMessage = (boost::format("line %d: invalid string >>> %s") % rowCol.first % subscrstr).str();
		return false;
	}
	std:: vector<TOKEN> tokens = tokenizer.getTokens();
	if (debugOption == "2") {
		std:: cout << "script tokens:" << std:: endl;
		for (size_t ti = 0; ti < tokens.size(); ++ti) {
			std:: vector<MYWCHAR_T> str = tokenizer.getTokenString(ti);
			std:: string s = toUTF8String(&str[0], str.size());
			std:: cout << s << std:: endl;
		}
		exit(0);
		return false; // dummy
	}
	
	// parse the script
	TorqParserTracer tracer;
	TorqParser parser;
	parser.setTokens(tokens);
	if (! parser.parse(&tracer)) {
		boost::int32_t errorTokenPos = parser.getErrorPos();
		TOKEN &errorToken = tokens[errorTokenPos];
		boost::int32_t errorPos = errorToken.beginPos;
		std:: pair<boost::int32_t, boost::int32_t> rowCol = posToRowCol(script, errorPos);
		std:: vector<MYWCHAR_T> subscr;
		size_t p = errorPos + 20;
		if (p > script.size()) {
			p = script.size();
		}
		subscr.insert(subscr.end(), script.begin() + errorPos, script.begin() + p);
		std:: string subscrstr = toUTF8String(&subscr[0], subscr.size());
		*pErrorMessage = (boost::format("line %d: parse error >>> %s") % rowCol.first % subscrstr).str();
		return false;
	}

	{
		std:: vector<TRACE_ITEM> v = tracer.getTrace();
		trace.swap(v);
	}
	{
		std:: vector<TRACE_ITEM> w = TorqParserTracer::reduce(trace);
		trace.swap(w);
	}
	if (debugOption == "3") {
		int r = debug_print_trace(trace, tokenizer);
		exit(r);
		return false; // dummy
	}

	return true;
}

class Main
{
private:
	std:: string scriptFile;
	std:: vector<std:: string> inputFiles;
	std:: string debugOption;
	std:: string printOption;
	std:: vector<std:: string> optionNodeFormats;
	std:: string outputFile;
	std:: string eachFileFormat;
	std:: vector<unsigned char> nodeNamesBinData;
	enum PrintFormat_t { PF_NORMAL, PF_XML, PF_CCFINDERNG } printFormat;
	boost::scoped_ptr<common::Encoder> pRawCharEncoder;
	boost::scoped_ptr<common::Encoder> pGeneratedEncoder;
	size_t cutoffValue;

	// used in analyzeCommandlineArgument and analyzeCommandLineArguments
	bool optionNSpecified;
	bool optionPXSpecified;

protected:
	std:: string usageFormatStr;
	boost::scoped_ptr<FileReader> pInputFileReader;

public:
	Main()
		: pRawCharEncoder(), pGeneratedEncoder(), 
		pInputFileReader(),
		usageFormatStr( 
			"Torq ver. %s (c) 2009-2010 AIST" "\n"
			"Usage: torq [option...] script input..." "\n"
			"Option" "\n"
			"  -c i:encoding: specify input encoding." "\n"
			"  -c p: print out available INPUT encoding names." "\n"
			"  -c r:encoding: specify text output encoding (-c r:html)." "\n"
			"  -c t:encoding: specify token output encoding (-c t:utf8)." "\n"
			"  -d n: output a list of node names." "\n"
			"  -e fileformat: create an output for each input." "\n" 
			"  -n name,opening,closing: specify a format of the token." "\n"
			"  -n name=replacingstring: specify a replacing string of the token." "\n"
			"  -n name:{flatten|none|terminate}: specify a format of the token." "\n"
			"  -o output: specify the output file." "\n"
			"  -p Nns: print options." "\n"
			"  -p x[Nn]: print in XML-like format and the options." "\n"
			"  -t number: interpreter cutoff value (10000)." "\n"
		)
	{
		EncodedFileReader *pefr = new EncodedFileReader();
		pefr->setEncoding("utf8");
		boost::scoped_ptr<FileReader> spefr(pefr);
		pInputFileReader.swap(spefr);
	}
	virtual ~Main()
	{
	}
private:
	Main &operator=(const Main &right)
	{
		assert(false); // not implemented
	}
protected:
	virtual int analyzeCommandlineArgument(const std:: vector<std:: string> &args, size_t *pIndex)
	{
		size_t &i = *pIndex;
		std:: string argi = args[i];
		if ((! argi.empty()) && argi[0] == '-') {
			if (argi == "-h" || argi == "--help" || argi == "-?") {
				std:: cout << (boost::format(usageFormatStr) % appVersion.toString()).str();
				exit(0);
				return 0;
			}
			else if (argi == "-c" || argi == "--encoding") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --encoding option requires an argument" << std:: endl;
					return 1;
				}
				std:: string str = args[i + 1];
				++i;
				if (str == "p") {
					std:: vector<std:: string> names = Decoder().getAvailableEncodings();
					for (size_t i = 0; i < names.size(); ++i) {
						std:: cout << names[i] << std:: endl;
					}
					exit(0);
					return 0;
				}
				if (str.length() < 2) {
					std:: cerr << "error: invalid --encoding argument" << std:: endl;
					return 1;
				}
				std:: string directive = str.substr(0, 2);
				std:: string name = str.substr(2);
				if (directive == "r:" || directive == "t:") {
					common::Encoder *pEncoder = NULL;
					if (name == "html") {
						pEncoder = new common::HTMLEncoder();
					}
					else if (name == "utf8n" || name == "utf8") {
						pEncoder = new common::UTF8NEncoder();
					}
					else {
						std:: cerr << "error: invalid --encoding argument" << std:: endl;
						return 1;
					}
					if (directive == "r:") {
						boost::scoped_ptr<common::Encoder> sp(pEncoder);
						pRawCharEncoder.swap(sp);
					}
					else if (directive == "t:") {
						boost::scoped_ptr<common::Encoder> sp(pEncoder);
						pGeneratedEncoder.swap(sp);
					}
				}
				else if (directive == "i:") {
					EncodedFileReader *pefr = new EncodedFileReader();
					if (! (*pefr).setEncoding(name)) {
						std:: cerr << "error: unsupported encoding '" << name << "'" << std::endl;
						return 1;
					}
					boost::scoped_ptr<FileReader> sp(pefr);
					pInputFileReader.swap(sp);
				}
				else {
					std:: cerr << "error: invalid --encoding argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-d" || argi == "--debug") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --debug option requires an argument" << std:: endl;
					return 1;
				}
				debugOption = args[i + 1];
				++i;
			}
			else if (argi == "-i" || argi == "--inputfiles") {
				if (i + 1 < args.size()) {
					std:: string listFileName = args[i + 1];
					if (! get_raw_lines(listFileName, &inputFiles)) {
						std:: cerr << "error: can not open a file '" << listFileName << "'" << std:: endl;
						return 1;
					}
					++i;
				}
				else {
					std:: cerr << "error: option --inputfiles requres an argument" << std:: endl;
					return 1;
				}
			}
			else if (argi == "-n" || argi == "--nodeformat") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --nodeformat option requires an argument" << std:: endl;
					return 1;
				}
				if (optionPXSpecified) {
					std:: cerr << "error: option --nodeformat and --print x are exclusive" << std:: endl;
					return 1;
				}
				optionNSpecified = true;
				std:: vector<MYWCHAR_T> label;
				text::Helper::NodeFormat openClose;
				if (! scan_option_node_format(args[i + 1], &label, &openClose)) {
					std:: cerr << "error: invalid --nodeformat argument '" << args[i] << "'" << std:: endl;
					return 1;
				}
				optionNodeFormats.push_back(args[i + 1]);
				++i;
			}
			else if (argi == "-o" || argi == "--output") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --output option requires an argument" << std:: endl;
					return 1;
				}
				if (! outputFile.empty()) {
					std:: cerr << "error: --output option was specified twice" << std:: endl;
					return 1;
				}
				outputFile = args[i + 1];
				++i;
			}
			else if (argi == "-p" || argi == "--print") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --print option requires an argument" << std:: endl;
					return 1;
				}
				printOption = args[i + 1];
				if (printOption.size() >= 1) {
					if (printOption[0] == 'x') {
						if (optionNSpecified) {
							std:: cerr << "error: option --nodeformat and --print x are exclusive" << std:: endl;
							return 1;
						}
						optionPXSpecified = true;
						printFormat = PF_XML;
						printOption = printOption.substr(1);
					}
					else if (printOption[0] == 'c') {
						printFormat = PF_CCFINDERNG;
						printOption = printOption.substr(1);
					}
				}
				++i;
			}
			else if (argi == "-e" || argi == "--each") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --each option requires an argument" << std:: endl;
					return 1;
				}
				eachFileFormat = args[i + 1];
				if (eachFileFormat.find("%s") == std:: string::npos) {
					std:: cerr << "error: --each argument has to include %s." << std:: endl;
					return 1;
				}
				if (eachFileFormat.find("%%") != std:: string::npos) {
					std:: cerr << "error: invalid --each argument." << std:: endl;
					return 1;
				}
				++i;
			}
			else if (argi == "-e" || argi == "--cutoff") {
				if (! (i + 1 < args.size())) {
					std:: cerr << "error: --cutoff option requires an argument" << std:: endl;
					return 1;
				}
				std:: string str = args[i + 1];
				char *p;
				boost::int32_t value = strtol(str.c_str(), &p, 10);
				if (p != NULL && (p - str.c_str()) != str.length()) {
					std:: cerr << "error: invalid --cutoff argument." << std:: endl;
					return 1;
				}
				cutoffValue = value;
				++i;
			}
			else {
				std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
				return 1;
			}
		}
		else if (scriptFile.empty()) {
			scriptFile = argi;
		}
		else {
			inputFiles.push_back(argi);
		}
		return 0;
	}
private:
	int analyzeCommandlineArguments(const std:: vector<std:: string> &args)
	{
		optionNSpecified = false;
		optionPXSpecified = false;
		cutoffValue = 10000;
		
		// analyze command-line arguments
		if (args.size() == 1) {
			std:: cout << (boost::format(usageFormatStr) % appVersion.toString()).str();
			exit(0);
			return 0;
		}
		for (size_t i = 1; i < args.size(); ++i) {
			int r = analyzeCommandlineArgument(args, &i);
			if (r != 0) {
				return r;
			}
		}
		if (scriptFile.empty()) {
			std:: cerr << "error: no script file is specified" << std:: endl;
			return 1;
		}
		
		return 0; // no error
	}
	void do_output_i(std:: ostream *pOutput, const text::TokenSequence &text,
		const std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormats,
		const std:: vector<std:: vector<MYWCHAR_T> > &nodeNames)
	{
		unsigned long options = 0;
		{
			bool notExpandNull = printOption.find('n') != std:: string::npos;
			bool notPrintNull = printOption.find('N') != std:: string::npos;
			options = 
					(notExpandNull ? 0 : text::Helper::RecurseNull) |  
					(notPrintNull ? text::Helper::SkipNull : 0);
			std:: vector<MYWCHAR_T> eol;
			common::EscapeSequenceHelper::decode(&eol, "eol");
			if (nodeFormats.find(eol) == nodeFormats.end()) {
				options |= text::Helper::NewLineThru;
			}
		}

		if (printFormat == PF_XML) {
			print_result_xml(pOutput, options, nodeFormats, nodeNames, text);
		}
		else if (printFormat == PF_CCFINDERNG) {
			print_result_cng(pOutput, nodeFormats, nodeNames, text);
		}
		else {
			print_result(pOutput, options, nodeFormats, nodeNames, pRawCharEncoder.get(), pGeneratedEncoder.get(), text);
		}
	}
	int do_output(const std:: string &outputFile, const text::TokenSequence &text,
		const std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormats,
		const std:: vector<std:: vector<MYWCHAR_T> > &nodeNames)
	{
		// print out the result
		std:: string tempFileName = make_temp_file_on_the_same_directory(outputFile, "torqtemp", ".tmp");
		if (! outputFile.empty()) {
			if (printFormat == PF_CCFINDERNG) {
#if defined _MSC_VER
				HANDLE hFile = openCompressedFileForWrite(tempFileName.c_str());
				if (hFile == INVALID_HANDLE_VALUE) {
					std:: cerr << "error: can not create a temporary file" << std:: endl;
					return 2;
				}
				{
					win32::basic_win32handle_ostream<char> ofs(hFile);
					do_output_i(&ofs, text, nodeFormats, nodeNames);
				}
				CloseHandle(hFile);
#else
				std:: ofstream ofs(tempFileName.c_str(), std:: ios::out | std::ios::trunc); // The output file is open in text mode. This makes a new-line character into \r\n on windows.
				if (! ofs.good()) {
					std:: cerr << "error: can not create a temporary file" << std:: endl;
					return 2;
				}
				do_output_i(&ofs, text, nodeFormats, nodeNames);
#endif
			}
			else {
				std:: ofstream ofs(tempFileName.c_str(), std:: ios::out | std::ios::trunc); // The output file is open in text mode. This makes a new-line character into \r\n on windows.
				if (! ofs.good()) {
					std:: cerr << "error: can not create a temporary file" << std:: endl;
					return 2;
				}
				do_output_i(&ofs, text, nodeFormats, nodeNames);
			}

			remove(outputFile.c_str());
			int r = rename(tempFileName.c_str(), outputFile.c_str());
			if (r != 0) {
				std:: cerr << "error: can not create file '" << outputFile << "'" << std:: endl;
				return -2;
			}
		}
		else {
			do_output_i(&std:: cout, text, nodeFormats, nodeNames);
		}
		return 0; // no error
	}
public:
	int main(int argc, char *argv[])
	{
		std:: vector<std:: string> args;
		args.insert(args.end(), &argv[0], &argv[argc]);
		return main(args);
	}
	int main(int argc, const char *argv[])
	{
		std:: vector<std:: string> args;
		args.insert(args.end(), &argv[0], &argv[argc]);
		return main(args);
	}
	int main(const std:: vector<std:: string> args)
	{
		{
			boost::scoped_ptr<common::Encoder> sp;
			pRawCharEncoder.swap(sp);
		}
		{
			boost::scoped_ptr<common::Encoder> sp;
			pGeneratedEncoder.swap(sp);
		}

		int r = analyzeCommandlineArguments(args);
		if (r != 0) {
			return r;
		}
		
		if (debugOption == "0") {
			std:: cout << "sizeof(text::RawCharToken) = " << sizeof(text::RawCharToken) << std:: endl;
			return 0;
		}

		// check the input files
		if (debugOption != "n") {
			if (inputFiles.size() == 0) {
				std:: cerr << "error: no input files are specified" << std:: endl;
				return 1;
			}
		}

		// generate node formats by command-line option
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormats;
		for (size_t i = 0; i < optionNodeFormats.size(); ++i) {
			const std:: string &f = optionNodeFormats[i];
			std:: vector<MYWCHAR_T> label;
			text::Helper::NodeFormat openClose;
			if (! scan_option_node_format(f, &label, &openClose)) {
				std:: cerr << "error: invalid --nodeformat argument '" << f << "'" << std:: endl;
				return 1;
			}
			nodeFormats[label].swap(openClose);
		}
		
		// read the script from file
		std:: vector<MYWCHAR_T> script;
		if (! read(&script, scriptFile)) {
			std:: cerr << "error: can not open file '" << scriptFile << "'" << std:: endl;
			return 1;
		}

		// parse the script
		std:: vector<TRACE_ITEM> trace;
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormatsInScript;
		std:: string errorMessage;
		if (! parse_script(script, &trace, &nodeFormatsInScript, &errorMessage, debugOption)) {
			std:: cerr << "error: " << errorMessage << std:: endl;
			return 1;
		}
		
		// merge node formats from script and from command-line 
		{
			for (std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::iterator i = nodeFormatsInScript.begin();
					i != nodeFormatsInScript.end(); ++i) {
				if (nodeFormats.find(i->first) == nodeFormats.end()) {
					nodeFormats[i->first].swap(i->second);
				}
			}
			nodeFormatsInScript.clear();
		}

		// prepare interpreter
		Interpreter interp;
		interp.setProgram(trace, script);
		std:: vector<std:: vector<MYWCHAR_T> > nodeNames = interp.getLabelStrings();
		std:: vector<MYWCHAR_T> varName;
		common::EscapeSequenceHelper::decode(&varName, "TEXT");
		if (debugOption == "n") {
			for (size_t i = 0; i < nodeNames.size(); ++i) {
				std:: cout << common::EscapeSequenceHelper::encode(nodeNames[i], false) << std:: endl;
			}
			return 0;
		}

		if (eachFileFormat.empty()) {
			// concat input files
			text::TokenSequence text;
			{
				std:: vector<MYWCHAR_T> seq;

				for (size_t i = 0; i < inputFiles.size(); ++i) {
					seq.resize(0);
					if (! (*pInputFileReader).read(&seq, inputFiles[i])) {
						std:: cerr << "error: " << (*pInputFileReader).getErrorMessage() << std:: endl;
						return 1;
					}
					text::TokenSequence fileText;
					text::Helper::buildTokenSequence(&fileText, seq, true, true);
					size_t curSize = text.size();
					text.resize(curSize + fileText.size());
					for (size_t j = 0; j < fileText.size(); ++j) {
						text::Token *pOld = text.replaceAt(curSize + j, fileText.replaceAt(j, NULL));
						if (pOld != NULL) {
							pOld->destroy();
						}
					}
				}
			}
			if (debugOption == "4") {
				std:: vector<MYWCHAR_T> separator;
				separator.push_back(' ');
				text::Helper::print(&std:: cout, text, separator, 0, NULL);
				return 0;
			}

			// do interpretation
			interp.setCutoffValue(cutoffValue * (long long)text.size());
			interp.swapVariable(varName, &text);
			boost::int32_t errorPc = interp.interpret(0);
			if (errorPc > 0) {
				if (inputFiles.size() == 1) {
					std:: cerr << "error: PC " << errorPc << ": interpretation error, code=" << (100 + interp.getError().code) << ", file=" << inputFiles[0] << std:: endl;
				}
				else {
					std:: cerr << "error: PC " << errorPc << ": interpretation error, code=" << (100 + interp.getError().code) << "" << std:: endl;
				}
				return 100 + interp.getError().code;
			}

			// output the result
			interp.swapVariable(varName, &text);
			r = do_output(outputFile, text, nodeFormats, nodeNames);
			if (r != 0) {
				return r;
			}
		}
		else {
			std:: vector<MYWCHAR_T> seq;
			text::TokenSequence text;
			for (size_t fi = 0; fi < inputFiles.size(); ++fi) {
				// read a input file
				if (! (*pInputFileReader).read(&seq, inputFiles[fi])) {
					std:: cerr << "error: " << (*pInputFileReader).getErrorMessage() << std:: endl;
					return 1;
				}
				text::Helper::buildTokenSequence(&text, seq, true, true);

				// do interpretation
				interp.swapVariable(varName, &text);
				boost::int32_t errorPc = interp.interpret(0);
				if (errorPc > 0) {
					std:: cerr << "error: PC " << errorPc << ": interpretation error, code=" << (100 + interp.getError().code) << ", file=" << inputFiles[fi] << std:: endl;
					return 100 + interp.getError().code;
				}

				// output the result
				interp.setCutoffValue(cutoffValue * (long long)text.size());
				interp.swapVariable(varName, &text);
				std:: string fileName = (boost::format(eachFileFormat) % inputFiles[fi]).str();
				r = do_output(fileName, text, nodeFormats, nodeNames);
				if (r != 0) {
					return r;
				}
				
				seq.resize(0);
				text.resize(0);
			}
		}

		return 0;
	}
};

#if defined BUILD_TORQDLL

#include "torqdll.h"

__declspec(dllexport) int WINAPI torq_main(int argc, const char *argv[])
{
	return Main().main(argc, argv);
}

__declspec(dllexport) void WINAPI torq_app_version(int *pMajor, int *pMin0, int *pMin1)
{
	*pMajor = appVersion.maj;
	*pMin0 = appVersion.min0;
	*pMin1 = appVersion.min1;
}

#elif defined BUILD_TORQEXE || defined BUILD_TORQ_MULTITHREADEXE

int main(int argc, char *argv[])
{
	return Main().main(argc, argv);
}

#endif

namespace easytorq {

	class RuntimeError : public std::runtime_error {
public:
	RuntimeError(const std::string &causeStr)
		: std::runtime_error(causeStr)
	{
	}
};
class ParseError : public RuntimeError {
public:
	ParseError(const std::string &causeStr) 
		: RuntimeError(causeStr)
	{
	}
};
class InterpretationError : public RuntimeError {
public:
	InterpretationError(const std::string &causeStr)
		: RuntimeError(causeStr)
	{
	}
};


class Tree {
private:
	text::TokenSequence text;
public:
	Tree(const std::string &utf8str)
	{
		std::vector<MYWCHAR_T> buf;
		toWStringV(&buf, utf8str);

		text::Helper::buildTokenSequence(&text, buf, true, true);
	}
	const text::TokenSequence *refText() const
	{
		return &text;
	}
	text::TokenSequence *refText()
	{
		return &text;
	}
};

class Pattern {
private:
	Interpreter interp;
	std:: vector<std:: vector<MYWCHAR_T> > nodeNames;
	std:: vector<MYWCHAR_T> varName;
	long cutoffValue;

public:
	Pattern(const std::string &patternStr)
		: cutoffValue(1000)
	{
		// read the script from file
		std:: vector<MYWCHAR_T> script;
		toWStringV(&script, patternStr);

		// parse the script
		std:: vector<TRACE_ITEM> trace;
		std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormatsInScript;
		std:: string errorMessage;
		if (! parse_script(script, &trace, &nodeFormatsInScript, &errorMessage, ""/* no debug option */)) {
			throw ParseError(errorMessage);
		}
		if (! nodeFormatsInScript.empty()) {
			throw ParseError("node format in script is not implemented");
		}
		
		// prepare interpreter
		interp.setProgram(trace, script);
		std:: vector<std:: vector<MYWCHAR_T> > nodeNames = interp.getLabelStrings();
		common::EscapeSequenceHelper::decode(&varName, "TEXT");
	}
	void setCutoffValue(long newValue)
	{
		cutoffValue = newValue;
	}
public:
	void apply(Tree *pTree) const
	{
		// do interpretation
		text::TokenSequence &text = *pTree->refText();
		Interpreter &interp = const_cast<Pattern *>(this)->interp;
		interp.setCutoffValue(cutoffValue * (long long)text.size());
		interp.swapVariable(varName, &text);
		boost::int32_t errorPc = interp.interpret(0);
		if (errorPc > 0) {
			interp.swapVariable(varName, &text);
			throw InterpretationError((boost::format("PC = %d, ErrorCode = %d") % errorPc % (100 + interp.getError().code)).str());
		}
		else {
			interp.swapVariable(varName, &text);
		}
	}
};

class FormatterBase {
public:
	virtual ~FormatterBase() { }
public:
	virtual void addNodeFlatten(const std::string &nodeName) = 0;
	virtual void addNodeNone(const std::string &nodeName) = 0;
	virtual void addNodeTerminate(const std::string &nodeName) = 0;
	virtual void addNodeReplace(const std::string &nodeName, const std::string &newName) = 0;
	virtual void addNodeFormat(const std::string &nodeName, const std::string &openStr, const std::string &closeStr) = 0;
	virtual std::string format(const Tree &tree) const = 0;
};

class Formatter : public FormatterBase {
private:
	std::map<std::vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormats;
public:
	void addNodeFlatten(const std::string &nodeName)
	{
		std::vector<MYWCHAR_T> nameUcs4 = toWStringV(nodeName);

		text::Helper::NodeFormat openClose;
		openClose.nodeType = text::Helper::NF_EXPANDED;

		nodeFormats[nameUcs4] = openClose;
	}
	void addNodeNone(const std::string &nodeName)
	{
		std::vector<MYWCHAR_T> nameUcs4 = toWStringV(nodeName);

		text::Helper::NodeFormat openClose;
		openClose.nodeType = text::Helper::NF_NONE;

		nodeFormats[nameUcs4] = openClose;
	}
	void addNodeTerminate(const std::string &nodeName)
	{
		std::vector<MYWCHAR_T> nameUcs4 = toWStringV(nodeName);

		text::Helper::NodeFormat openClose;
		openClose.nodeType = text::Helper::NF_TERMINATED;
		openClose.opening = nameUcs4;

		nodeFormats[nameUcs4] = openClose;
	}
	void addNodeReplace(const std::string &nodeName, const std::string &newName)
	{
		std::vector<MYWCHAR_T> nameUcs4 = toWStringV(nodeName);

		std::vector<MYWCHAR_T> newNameUcs4;
		common::EscapeSequenceHelper::decode(&newNameUcs4, newName);

		text::Helper::NodeFormat openClose;
		openClose.nodeType = text::Helper::NF_TERMINATED;
		openClose.opening = newNameUcs4;

		nodeFormats[nameUcs4] = openClose;
	}
	void addNodeFormat(const std::string &nodeName, const std::string &openStr, const std::string &closeStr)
	{
		std::vector<MYWCHAR_T> nameUcs4 = toWStringV(nodeName);

		std::vector<MYWCHAR_T> openUcs4;
		common::EscapeSequenceHelper::decode(&openUcs4, openStr);
		std::vector<MYWCHAR_T> closeUcs4;
		common::EscapeSequenceHelper::decode(&closeUcs4, closeStr);

		text::Helper::NodeFormat openClose;
		openClose.nodeType = text::Helper::NF_EXPANDED;
		openClose.opening = openUcs4;
		openClose.closing = closeUcs4;

		nodeFormats[nameUcs4] = openClose;
	}
public:
	std::string format(const Tree &tree) const
	{
		std:: vector<std:: vector<MYWCHAR_T> > labelStrings = LabelCodeTableSingleton::instance()->getLabelStrings();
		HASH_MAP<boost::int32_t/* code */, text::Helper::NodeFormat> nfs;
		{
			std:: vector<MYWCHAR_T> closingLabel;
			for (size_t i = 0; i < labelStrings.size(); ++i) {
				const std:: vector<MYWCHAR_T> &label = labelStrings[i];
				assert(label.size() > 0);
				std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat>::const_iterator i0 = nodeFormats.find(label);
				if (i0 == nodeFormats.end()) {
					nfs[(boost::int32_t)i] = text::Helper::NodeFormat(text::Helper::NF_TERMINATED, label, closingLabel); // the default is "terminated"
				}
				else {
					nfs[(boost::int32_t)i] = i0->second;
				}
			}
		}

		std::basic_ostringstream<char> output;
		text::Helper::printCng(&output, *tree.refText(), nfs);
		return output.str();
	}
};

};

