#include <string>
#include <vector>
#include <utility>
#include <map>
#include "../../common/hash_map_includer.h"

#include <boost/format.hpp>
#include <boost/cstdint.hpp>

#include "../torqcommon.h"
#include "../torqtokenizer.h"
#include "../torqparser.h"
#include "../texttoken.h"
#include "../interpreter.h"

#include "easytorq.h"

namespace {

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

bool parse_script(const std:: vector<MYWCHAR_T> &script, std:: vector<TRACE_ITEM> *pTrace, 
	std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> *pNodeFormatsInScript, 
	std:: string *pErrorMessage)
{
	assert(pTrace != NULL);
	assert(pNodeFormatsInScript != NULL);
	assert(pErrorMessage != NULL);

	std:: vector<TRACE_ITEM> &trace = *pTrace;
	std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> &nodeFormatsInScript = *pNodeFormatsInScript;

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

	return true;
}

};

namespace easytorq {

Tree::Tree(const std::string &utf8str)
{
	std::vector<MYWCHAR_T> buf;
	toWStringV(&buf, utf8str);

	text::Helper::buildTokenSequence(&text, buf, true, true);
}

const text::TokenSequence *Tree::refText() const
{
	return &text;
}

text::TokenSequence *Tree::refText()
{
	return &text;
}

Pattern::Pattern(const std::string &patternStr)
	: cutoffValue(1000)
{
	// read the script from file
	std:: vector<MYWCHAR_T> script;
	toWStringV(&script, patternStr);

	// parse the script
	std:: vector<TRACE_ITEM> trace;
	std:: map<std:: vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormatsInScript;
	std:: string errorMessage;
	if (! parse_script(script, &trace, &nodeFormatsInScript, &errorMessage)) {
		throw ParseError(errorMessage);
	}
	if (! nodeFormatsInScript.empty()) {
		throw ParseError("node format in script is not implemented");
	}
	
	// prepare interpreter
	interp.setProgram(trace, script);
	common::EscapeSequenceHelper::decode(&varName, "TEXT");
}

void Pattern::setCutoffValue(long newValue)
{
	cutoffValue = newValue;
}

void Pattern::apply(Tree *pTree) const
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

void CngFormatter::addNodeFlatten(const std::string &nodeName)
{
	std::vector<MYWCHAR_T> nameUcs4;
	toWStringV(&nameUcs4, nodeName);

	text::Helper::NodeFormat openClose;
	openClose.nodeType = text::Helper::NF_EXPANDED;

	nodeFormats[nameUcs4] = openClose;
}

void CngFormatter::addNodeNone(const std::string &nodeName)
{
	std::vector<MYWCHAR_T> nameUcs4;
	toWStringV(&nameUcs4, nodeName);

	text::Helper::NodeFormat openClose;
	openClose.nodeType = text::Helper::NF_NONE;

	nodeFormats[nameUcs4] = openClose;
}

void CngFormatter::addNodeTerminate(const std::string &nodeName)
{
	std::vector<MYWCHAR_T> nameUcs4;
	toWStringV(&nameUcs4, nodeName);

	text::Helper::NodeFormat openClose;
	openClose.nodeType = text::Helper::NF_TERMINATED;
	openClose.opening = nameUcs4;

	nodeFormats[nameUcs4] = openClose;
}

void CngFormatter::addNodeReplace(const std::string &nodeName, const std::string &newName)
{
	std::vector<MYWCHAR_T> nameUcs4;
	toWStringV(&nameUcs4, nodeName);

	std::vector<MYWCHAR_T> newNameUcs4;
	common::EscapeSequenceHelper::decode(&newNameUcs4, newName);

	text::Helper::NodeFormat openClose;
	openClose.nodeType = text::Helper::NF_TERMINATED;
	openClose.opening = newNameUcs4;

	nodeFormats[nameUcs4] = openClose;
}

void CngFormatter::addNodeFormat(const std::string &nodeName, const std::string &openStr, const std::string &closeStr)
{
	std::vector<MYWCHAR_T> nameUcs4;
	toWStringV(&nameUcs4, nodeName);

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

std::string CngFormatter::format(const Tree &tree) const
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

#include <iostream>

#if defined EASYTORQ_TEST_MAIN

int main(int argc, char *argv[])
{
	using namespace easytorq;

	Tree tree("abc");
	Pattern pat("TEXT scan= x <- (\"a\" | \"b\" | \"c\");");
	pat.apply(&tree);

	CngFormatter formatter;
	std::cout << formatter.format(tree);

	return 0;
}

#endif // defined EASYTORQ_TEST_MAIN

