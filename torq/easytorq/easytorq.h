#if ! defined EASYTORQ_H
#define EASYTORQ_H

#include <stdexcept>
#include <string>
#include <vector>
#include <map>

#include "../../common/utf8support.h"

#include "../interpreter.h"

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
	Tree(const std::string &utf8str);
	const text::TokenSequence *refText() const;
	text::TokenSequence *refText();
};

class Pattern {
private:
	Interpreter interp;
	std:: vector<MYWCHAR_T> varName;
	long cutoffValue;

public:
	Pattern(const std::string &patternStr); // throws ParseError
	void setCutoffValue(long newValue);
	void apply(Tree *pTree) const; // throws InterpretationError
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

class CngFormatter : public FormatterBase {
private:
	std::map<std::vector<MYWCHAR_T>, text::Helper::NodeFormat> nodeFormats;
public:
	void addNodeFlatten(const std::string &nodeName);
	void addNodeNone(const std::string &nodeName);
	void addNodeTerminate(const std::string &nodeName);
	void addNodeReplace(const std::string &nodeName, const std::string &newName);
	void addNodeFormat(const std::string &nodeName, const std::string &openStr, const std::string &closeStr);
	std::string format(const Tree &tree) const;
};

};

#endif // EASYTORQ_H
