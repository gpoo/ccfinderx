#include <map>
#include "../common/hash_map_includer.h"
#include <string>

#include <boost/cstdint.hpp>

#include "../common/utf8support.h"

#include "torqcommon.h"

bool common::substrEqual(const std:: vector<MYWCHAR_T> &str, boost::int32_t pos, const std:: string &token)
{
	if (! (pos + token.length() <= str.size())) {
		return false;
	}
	
	for (size_t i = 0; i < token.length(); ++i) {
		if (str[pos + i] != (MYWCHAR_T)(token[i])) {
			return false;
		}
	}

	return true;	
}

boost::int32_t/* pos */ common::substrFind(const std:: vector<MYWCHAR_T> &str, boost::int32_t pos, const std:: string &token)
{
	if (token.empty()) {
		return pos; // found
	}

	MYWCHAR_T ch0 = (MYWCHAR_T)(token[0]);

	boost::int32_t p = pos;
	while (true) {
		while (p < str.size() && str[p] != ch0) {
			++p;
		}
		if (! (p < str.size())) {
			return -1; // not found
		}
		assert(str[p] == ch0);
		if (substrEqual(str, p, token)) {
			return p; // found
		}
		++p;
	}
	return -1; // dummy
}

namespace {

std:: pair<std:: string, MYWCHAR_T> specialCharTable[] = {
#include "specialchars.h"
	std:: pair<std:: string, MYWCHAR_T>("", 0)
};

std:: pair<std:: string, MYWCHAR_T> specialCharTableExtension[] = {
	std:: pair<std:: string, MYWCHAR_T>("&squot;", '\''),
	std:: pair<std:: string, MYWCHAR_T>("&bslash;", '\\'),
	std:: pair<std:: string, MYWCHAR_T>("&a;", '\a'),
	std:: pair<std:: string, MYWCHAR_T>("&b;", '\b'),
	std:: pair<std:: string, MYWCHAR_T>("&t;", '\t'),
	std:: pair<std:: string, MYWCHAR_T>("&r;", '\r'),
	std:: pair<std:: string, MYWCHAR_T>("&f;", '\f'),
	std:: pair<std:: string, MYWCHAR_T>("&n;", '\n'),
	std:: pair<std:: string, MYWCHAR_T>("&f;", '\f'),
	std:: pair<std:: string, MYWCHAR_T>("&v;", '\v'),
	std:: pair<std:: string, MYWCHAR_T>("", 0)
};

std:: map<std:: string, MYWCHAR_T> makeStrToCodeTable()
{
	std:: map<std:: string, MYWCHAR_T> ctbl;

	{
		for (size_t i = 0; ! specialCharTable[i].first.empty(); ++i) {
			ctbl.insert(specialCharTable[i]);
		}
	}
	{
		for (size_t i = 0; ! specialCharTableExtension[i].first.empty(); ++i) {
			ctbl.insert(specialCharTableExtension[i]);
		}
	}
	return ctbl;
}

HASH_MAP<MYWCHAR_T, std:: string> makeCodeToStrTable()
{
	HASH_MAP<MYWCHAR_T, std:: string> ctbl;

	{
		for (size_t i = 0; ! specialCharTable[i].first.empty(); ++i) {
			ctbl[specialCharTable[i].second] = specialCharTable[i].first;
		}
	}
	{
		for (size_t i = 0; ! specialCharTableExtension[i].first.empty(); ++i) {
			ctbl[specialCharTableExtension[i].second] = specialCharTableExtension[i].first;
		}
	}
	return ctbl;
}

HASH_MAP<MYWCHAR_T, std:: string> makeCodeToStrTableExplicit()
{
	HASH_MAP<MYWCHAR_T, std:: string> ctbl;

	for (size_t i = 0; specialCharTable[i].first.length() > 0; ++i) {
		ctbl[specialCharTable[i].second] = specialCharTable[i].first;
	}
	return ctbl;
}

}; // namespace

const std:: map<std:: string, MYWCHAR_T> common::EscapeSequenceHelper::strToCodeTable(makeStrToCodeTable());
const HASH_MAP<MYWCHAR_T, std:: string> common::EscapeSequenceHelper::codeToStrTable(makeCodeToStrTable());
const HASH_MAP<MYWCHAR_T, std:: string> common::EscapeSequenceHelper::codeToStrTableExplicit(makeCodeToStrTableExplicit());


