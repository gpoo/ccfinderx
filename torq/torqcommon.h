#if ! defined TORQCOMMON_H
#define TORQCOMMON_H

#include <cassert>
#include <cstdio>
#include <string>

#include <map>
#include "../common/hash_map_includer.h"

#include <boost/format.hpp>
#include <boost/cstdint.hpp>
#include <boost/assign/std/vector.hpp>

#include "../common/utf8support.h"

namespace common {

bool substrEqual(const std:: vector<MYWCHAR_T> &str, boost::int32_t pos, const std:: string &token);
boost::int32_t/* pos */ substrFind(const std:: vector<MYWCHAR_T> &str, boost::int32_t pos, const std:: string &token);

class EscapeSequenceHelper
{
private:
	static const std:: map<std:: string, MYWCHAR_T> strToCodeTable;
	static const HASH_MAP<MYWCHAR_T, std:: string> codeToStrTable;
	static const HASH_MAP<MYWCHAR_T, std:: string> codeToStrTableExplicit;
public:
	static std:: string encode(MYWCHAR_T c, bool explicitHTML)
	{
		const HASH_MAP<MYWCHAR_T, std:: string> &table = explicitHTML ? codeToStrTableExplicit : codeToStrTable;
		std:: string r;
		HASH_MAP<MYWCHAR_T, std:: string>::const_iterator ti = table.find(c);
		if (ti != table.end()) {
			r += ti->second;
		}
		else {
			if (0 <= c && c < 0x20) {
				r += (boost::format("&#x%x;") % (unsigned)c).str();
			}
			else if (c <= 0x7f) {
				r += (char)c;
			}
			else {
				r += (boost::format("&#x%lx;") % c).str();
			}
		}
		return r;
	}
	static std:: string encode(const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end, bool explicitHTML)
	{
		const HASH_MAP<MYWCHAR_T, std:: string> &table = explicitHTML ? codeToStrTableExplicit : codeToStrTable;
		assert(begin >= 0 && begin <= text.size());
		assert(end >= 0 && end <= text.size());

		std:: string r;
		//r.reserve(end - begin);
		for (size_t i = begin; i < end; ++i) {
			MYWCHAR_T c = text[i];
			HASH_MAP<MYWCHAR_T, std:: string>::const_iterator ti = table.find(c);
			if (ti != table.end()) {
				r += ti->second;
			}
			else {
				if (0 <= c && c < 0x20) {
					r += (boost::format("&#x%x;") % (unsigned)c).str();
				}
				else if (c <= 0x7f) {
					r += (char)c;
				}
				else {
					r += (boost::format("&#x%lx;") % c).str();
				}
			}
		}
		return r;
	}
	static std:: string encode(const std:: vector<MYWCHAR_T> &text, bool explicitHTML)
	{
		return encode(text, 0, text.size(), explicitHTML);
	}
	static void decode(std:: vector<MYWCHAR_T> *pResult, const std:: string &text, boost::int32_t begin, boost::int32_t end)
	{
		assert(begin >= 0 && begin <= text.size());
		assert(end >= 0 && end <= text.size());

		(*pResult).clear();
		std:: vector<MYWCHAR_T> &r = *pResult;
		//r.reserve(end - begin);
		size_t i = begin; 
		while (i < end) {
			char c = text[i];
			if (c == '&') {
				if (i + 1 < end && text[i + 1] == '#') {
					if (i + 2 < end && text[i + 2] == 'x') {
						size_t j = i + 3;
						char d;
						while (j < end && ('a' <= (d = text[j]) && d <= 'f' || 'A' <= d && d <= 'F' || '0' <= d && d <= '9')) {
							++j;
						}
						if (j < end && text[j] == ';') {
							MYWCHAR_T x;
							sscanf(text.substr(i + 3, j - (i + 3)).c_str(), "%lx", &x);
							r.push_back(x);
							i = j;
						}
						else {
							r.push_back((MYWCHAR_T)c);
							++i;
						}
					}
					else {
						size_t j = i + 3;
						char d;
						while (j < end && ('0' <= (d = text[j]) && d <= '9')) {
							++j;
						}
						if (j < end && text[j] == ';') {
							MYWCHAR_T d;
							sscanf(text.substr(i + 3, j - (i + 3)).c_str(), "%ld", &d);
							r.push_back(d);
							i = j;
						}
						else {
							r.push_back((MYWCHAR_T)c);
							++i;
						}
					}
				}
				else {
					size_t j = i + 1;
					char d;
					while (j < end && ('a' <= (d = text[j]) && d <= 'z' || 'A' <= d && d <= 'Z' || '0' <= d && d <= '9')) {
						++j;
					}
					if (j < end && text[j] == ';') {
						std:: string escapeStr = text.substr(i, j - i);
						std:: map<std:: string, MYWCHAR_T>::const_iterator ti = strToCodeTable.find(escapeStr);
						if (ti != strToCodeTable.end()) {
							r.push_back(ti->second);
							i += escapeStr.length();
						}
						else {
							r.push_back((MYWCHAR_T)c);
							++i;
						}
					}
					else {
						r.push_back((MYWCHAR_T)c);
						++i;
					}
				}
			}
			else {
				r.push_back((MYWCHAR_T)c);
				++i;
			}
		}
	}
	static void decode(std:: vector<MYWCHAR_T> *pResult, const std:: string &text)
	{
		decode(pResult, text, 0, text.length());
	}
	static void decode(std:: vector<MYWCHAR_T> *pResult, const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end)
	{
		using namespace boost::assign; // bring 'operator+=()' into scope

		assert(begin >= 0 && begin <= text.size());
		assert(end >= 0 && end <= text.size());

		(*pResult).clear();
		std:: vector<MYWCHAR_T> &r = *pResult;
		//r.reserve(end - begin);
		size_t i = begin; 
		while (i < end) {
			MYWCHAR_T c = text[i];
			if (c == '&') {
				if (i + 1 < end && text[i + 1] == '#') {
					if (i + 2 < end && text[i + 2] == 'x') {
						if (i + 3 < end && text[i + 3] == '(') {
							// the text is "&#x(...
							size_t j = i + 4;
							std:: string buf;
							MYWCHAR_T d;
							while (j < end && ('a' <= (d = text[j]) && d <= 'f' || 'A' <= d && d <= 'F' || '0' <= d && d <= '9')) {
								buf += (char)d;
								++j;
							}
							if (j < end && text[j] == '-') {
								++j;
								std:: string buf2;
								while (j < end && ('a' <= (d = text[j]) && d <= 'f' || 'A' <= d && d <= 'F' || '0' <= d && d <= '9')) {
									buf2 += (char)d;
									++j;
								}
								if (j + 1 < end && text[j] == ')' && text[j + 1] == ';') {
									j += 2;
									char *p;
									MYWCHAR_T x = strtoul(buf.c_str(), &p, 16);
									MYWCHAR_T x2 = strtoul(buf2.c_str(), &p, 16);
									r += '&', '(', x, '-', x2, ')', ';';
									i = j;
								}
								else {
									r.push_back(c);
									++i;
								}
							}
							else {
								r.push_back(c);
								++i;
							}
						}
						else {
							// the text is "&#x...
							size_t j = i + 3;
							std:: string buf;
							MYWCHAR_T d;
							while (j < end && ('a' <= (d = text[j]) && d <= 'f' || 'A' <= d && d <= 'F' || '0' <= d && d <= '9')) {
								buf += (char)d;
								++j;
							}
							if (j < end && text[j] == ';') {
								char *p;
								MYWCHAR_T x = strtoul(buf.c_str(), &p, 16);
								r.push_back(x);
								i = j;
							}
							else {
								r.push_back(c);
								++i;
							}
						}
					}
					else {
						if (i + 2 < end && text[i + 2] == '(') {
							// the text is "&#(...
							size_t j = i + 3;
							std:: string buf;
							MYWCHAR_T d;
							while (j < end && ('0' <= (d = text[j]) && d <= '9')) {
								buf += (char)d;
								++j;
							}
							if (j < end && text[j] == '-') {
								++j;
								std:: string buf2;
								while (j < end && ('0' <= (d = text[j]) && d <= '9')) {
									buf2 += (char)d;
									++j;
								}
								if (j + 1 < end && text[j] == ')' && text[j + 1] == ';') {
									j += 2;
									char *p;
									MYWCHAR_T d = strtoul(buf.c_str(), &p, 10);
									MYWCHAR_T d2 = strtoul(buf2.c_str(), &p, 10);
									r += '&', '(', d, '-', d2, ')', ';';
									i = j;
								}
								else {
									r.push_back(c);
									++i;
								}
							}
							else  {
								r.push_back(c);
								++i;
							}
						}
						else {
							// the text is "&#...
							size_t j = i + 2;
							std:: string buf;
							MYWCHAR_T d;
							while (j < end && ('0' <= (d = text[j]) && d <= '9')) {
								buf += (char)d;
								++j;
							}
							if (j < end && text[j] == ';') {
								char *p;
								MYWCHAR_T d = strtoul(buf.c_str(), &p, 10);
								r.push_back(d);
								i = j;
							}
							else {
								r.push_back(c);
								++i;
							}
						}
					}
				}
				else {
					// the text is "&...
					size_t j = i + 1;
					std:: string buf;
					buf += "&";
					MYWCHAR_T d;
					while (j < end && ('a' <= (d = text[j]) && d <= 'z' || 'A' <= d && d <= 'Z' || '0' <= d && d <= '9')) {
						buf += (char)d;
						++j;
					}
					if (j < end && text[j] == ';') {
						buf += ";";
						std:: string escapeStr = buf;
						std:: map<std:: string, MYWCHAR_T>::const_iterator ti = strToCodeTable.find(escapeStr);
						if (ti != strToCodeTable.end()) {
							r.push_back(ti->second);
							i += escapeStr.length();
						}
						else {
							r.push_back(c);
							++i;
						}
					}
					else {
						r.push_back(c);
						++i;
					}
				}
			}
			else {
				r.push_back(c);
				++i;
			}
		}
	}
	static void decode(std:: vector<MYWCHAR_T> *pResult, const std:: vector<MYWCHAR_T> &text)
	{
		decode(pResult, text, 0, text.size());
	}
};

class Encoder
{
public:
	virtual ~Encoder()
	{
	}
	virtual std:: string encode(MYWCHAR_T c) const = 0;
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end) const = 0;
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text) const = 0;
};

class HTMLEncoder : public Encoder
{
public:
	virtual std:: string encode(MYWCHAR_T c) const
	{
		return EscapeSequenceHelper::encode(c, false);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end) const
	{
		return EscapeSequenceHelper::encode(text, begin, end, false);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text) const
	{
		return EscapeSequenceHelper::encode(text, false);
	}
};

class ExplicitHTMLEncoder : public Encoder
{
public:
	virtual std:: string encode(MYWCHAR_T c) const
	{
		return EscapeSequenceHelper::encode(c, true);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end) const
	{
		return EscapeSequenceHelper::encode(text, begin, end, true);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text) const
	{
		return EscapeSequenceHelper::encode(text, true);
	}
};

class UTF8NEncoder : public Encoder
{
public:
	virtual std:: string encode(MYWCHAR_T c) const
	{
		return toUTF8String(&c, 1);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text, boost::int32_t begin, boost::int32_t end) const
	{
		return toUTF8String(&text[begin], end - begin);
	}
	virtual std:: string encode(const std:: vector<MYWCHAR_T> &text) const
	{
		return toUTF8String(&text[0], text.size());
	}
};

struct Version {
public:
	typedef unsigned char binary_t[4];
public:
	int maj;
	int min0;
	int min1;
	int alphabet;
public:
	Version(int maj_, int min0_, int min1_, int alphabet_)
	{
		assert(0 <= maj_ && maj_ < 256);
		assert(0 <= min0 && min0 < 256);
		assert(0 <= min1 && min1 < 256);
		assert(alphabet_ == 0 || 'a' <= alphabet_ && alphabet_ <= 'z' || 'A' <= alphabet_ && alphabet_ <= 'Z');
		maj = maj_;
		min0 = min0_;
		min1 = min1_;
		alphabet = alphabet_;
	}
	Version(int maj_, int min0_, int min1_)
	{
		assert(0 <= maj_ && maj_ < 256);
		assert(0 <= min0 && min0 < 256);
		assert(0 <= min1 && min1 < 256);
		maj = maj_;
		min0 = min0_;
		min1 = min1_;
		alphabet = 0;
	}
	Version()
		: maj(0), min0(0), min1(0), alphabet(0)
	{
	}
	Version(const Version &right)
		: maj(right.maj), min0(right.min0), min1(right.min1), alphabet(right.alphabet)
	{
	}
	void swap(Version &right)
	{
		std:: swap(this->maj, right.maj);
		std:: swap(this->min0, right.min0);
		std:: swap(this->min1, right.min1);
		std:: swap(this->alphabet, right.alphabet);
	}
	std:: string toString() const
	{
		if (alphabet != 0) {
			return (boost::format("%d.%d.%d%c") % maj % min0 % min1 % alphabet).str();
		}
		else {
			return (boost::format("%d.%d.%d") % maj % min0 % min1).str();
		}
	}
	bool scan(const std:: string &str)
	{
		Version v;
		char c;
		int r = sscanf(str.c_str(), "%d.%d.%d%c", &v.maj, &v.min0, &v.min1, &c);
		v.alphabet = (int)c;
		if (r == 4 && 0 <= v.maj && v.maj < 256 
				&& 0 <= v.min0 && v.min0 < 256 
				&& 0 <= v.min1 && v.min1 < 256
				&& ('a' <= v.alphabet && v.alphabet <= 'z' || 'A' <= v.alphabet && v.alphabet <= 'Z')) {
			*this = v;
			return true;
		}
		v.alphabet = 0;
		r = sscanf(str.c_str(), "%d.%d.%d", &v.maj, &v.min0, &v.min1);
		if (0 <= v.maj && v.maj < 256 
				&& 0 <= v.min0 && v.min0 < 256 
				&& 0 <= v.min1 && v.min1 < 256) {
			*this = v;
			return true;
		}
		return false;
	}
	void write(binary_t *pBlock) const
	{
		binary_t &block = *pBlock;
		block[0] = (unsigned char)maj;
		block[1] = (unsigned char)min0;
		block[2] = (unsigned char)min1;
		block[3] = (unsigned char)alphabet;
	}
	bool read(const binary_t &block)
	{
		Version v;
		v.maj = (int)block[0];
		v.min0 = (int)block[1];
		v.min1 = (int)block[2];
		v.alphabet = (int)block[3];
		if (0 <= v.maj && v.maj < 256 
				&& 0 <= v.min0 && v.min0 < 256 
				&& 0 <= v.min1 && v.min1 < 256
				&& (v.alphabet == 0 || 'a' <= v.alphabet && v.alphabet <= 'z' || 'A' <= v.alphabet && v.alphabet <= 'Z')) {
			*this = v;
			return true;
		}
		return false;
	}
public:
	bool operator==(const Version &right) const
	{
		if (this->maj == right.maj) {
			if (this->min0 == right.min0) {
				if (this->min1 == right.min1) {
					if (this->alphabet == right.alphabet) {
						return true;
					}
				}
			}
		}
		return false;
	}
	bool operator<(const Version &right) const
	{
		if (this->maj < right.maj) {
			return true;
		}
		else if (this->maj == right.maj) {
			if (this->min0 < right.min0) {
				return true;
			}
			else if (this->min0 == right.min0) {
				if (this->min1 < right.min1) {
					return true;
				}
				else if (this->min1 == right.min1) {
					if (this->alphabet < right.alphabet) {
						return true;
					}
					else if (this->alphabet == right.alphabet) {
					}
				}
			}
		}
		return false;
	}
};

}; // namespace

#endif // TORQCOMMON_H
