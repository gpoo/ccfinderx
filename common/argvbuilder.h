#if ! defined ARGVBUILDER_H
#define ARGVBUILDER_H

#include <string>
#include <vector>

#include "unportable.h"

//std::string build_system_string(const std::vector<std::string> &argv);

class ArgvBuilder {
private:
	std::vector<std::string> argValues;
	std::vector<char> buffer;
	std::vector<size_t> argumentPositions;
	mutable std::vector<const char *> argCPtrs;
	mutable std::vector<char *> argPtrs;
public:

	/*
		This constructor is provided for converting the raw argc/argv to an ArgvBuilder object, like:
		int main(int argc, char *argv[]) {
			std::vector<std::string> args = ArgvBuilder(argv + 0, argv + argc).value();
			....
		}
	*/
	template<typename StringConvertibleIterator>
	ArgvBuilder(StringConvertibleIterator begin, StringConvertibleIterator end)
	{
		for (StringConvertibleIterator i = begin; i != end; ++i) {
			this->push_back(*i);
		}
	}

	inline ArgvBuilder()
	{
	}
	void push_back(const std::string &arg)
	{
		std::string s = escape_spaces(arg);
		argValues.push_back(s);
		argumentPositions.push_back(buffer.size());
		for (size_t i = 0; i < s.length(); ++i) {
			buffer.push_back(s[i]);
		}
		buffer.push_back('\0');
	}
	void clear()
	{
		argValues.clear();
		buffer.clear();
		argumentPositions.clear();
		argCPtrs.clear();
	}
	const char * const *c_argv() const
	{
		argCPtrs.clear();
		for (std::vector<size_t>::const_iterator i = argumentPositions.begin(); i < argumentPositions.end(); ++i) {
			size_t pos = *i;
			argCPtrs.push_back((const char *)(void *)&buffer[pos]);
		}
		argCPtrs.push_back(NULL);
		return &argCPtrs[0];
	}
	char * const *argv()
	{
		argPtrs.clear();
		for (std::vector<size_t>::const_iterator i = argumentPositions.begin(); i < argumentPositions.end(); ++i) {
			size_t pos = *i;
			argPtrs.push_back((char *)(void *)&buffer[pos]);
		}
		argPtrs.push_back(NULL);
		return &argPtrs[0];
	}
	inline const std::vector<std::string> &value() const
	{
		return argValues;
	}
	std::string str() const
	{
		std::string r;
		for (size_t i = 0; i < argValues.size(); ++i) {
			r += argValues[i];
			r += " ";
		}
		if (r.length() > 0) {
			r.erase(r.length() - 1);
		}
		return r;
	}	
};

#endif // defined ARGVBUILDER_H
