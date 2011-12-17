#include <sstream>
#include <assert.h>
#include "utf8support.h"
#include "allocaarray.h"

size_t countCharUTF8String(const char *str, size_t strLength)
{
	size_t count = 0;
	size_t i = 0;
	while (i < strLength) {
		++count;
		size_t nextI = nextCharUTF8String(str, strLength, i);
		if (nextI == i) {
			break; // invalid string
		}
		assert(nextI > i);
		i = nextI;
	}
	return count;
}

size_t countCharUTF8String(const std:: string &str)
{
	size_t count = 0;
	size_t i = 0;
	while (i < str.length()) {
		++count;
		size_t nextI = nextCharUTF8String(str, i);
		if (nextI == i) {
			break; // invalid string
		}
		assert(nextI > i);
		i = nextI;
	}
	return count;
}

#if defined WSTRING_CONVERSION_SUPPORT

size_t toUTF8String(char *buf, const MYWCHAR_T *str, size_t strLength)
{
	size_t j = 0;
	for (size_t i = 0; i < strLength; ++i) {
		int shift_count = -1;
		MYWCHAR_T c = str[i];
		if (c != 0x00 && c <= 0x7f) {
			buf[j++] = (char)(unsigned char)c;
		}
		else if (c <= 0x07ff) {
			unsigned char firstChar = 0xc0 | (unsigned char)(c >> 6);
			buf[j++] = firstChar;
			shift_count = 0;
		}
		else if (c <= 0xffff) {
			unsigned char firstChar = 0xe0 | (unsigned char)(c >> 12);
			buf[j++] = firstChar;
			shift_count = 6;
		}
		else if (c <= 0x001FFFFFUL) {
			unsigned char firstChar = 0xf0 | (unsigned char)(c >> 18);
			buf[j++] = firstChar;
			shift_count = 12;
		}
		else if (c <= 0x03FFFFFFUL) {
			unsigned char firstChar = 0xf8 | (unsigned char)(c >> 24);
			buf[j++] = firstChar;
			shift_count = 18;
		}
		else if (c <= 0x7FFFFFFFUL) {
			unsigned char firstChar = 0xfc | (unsigned char)(c >> 30);
			buf[j++] = firstChar;
			shift_count = 24;
		}
		else {
			assert(false);
		}
		while (shift_count >= 0) {
			unsigned char fc = 0x80 | (unsigned char)((c >> shift_count) & 0x3f);
			buf[j++] = fc;
			shift_count -= 6;
		}
	}

	return j;
}

std:: string toUTF8String(const MYWCHAR_T *str, size_t strLength)
{
	std:: string buf;
	for (size_t i = 0; i < strLength; ++i) {
		int shift_count = -1;
		MYWCHAR_T c = str[i];
		if (c != 0x00 && c <= 0x7f) {
			buf += (char)(unsigned char)c;
		}
		else if (c <= 0x07ff) {
			unsigned char firstChar = 0xc0 | (unsigned char)(c >> 6);
			buf += firstChar;
			shift_count = 0;
		}
		else if (c <= 0xffff) {
			unsigned char firstChar = 0xe0 | (unsigned char)(c >> 12);
			buf += firstChar;
			shift_count = 6;
		}
		else if (c <= 0x001FFFFFUL) {
			unsigned char firstChar = 0xf0 | (unsigned char)(c >> 18);
			buf += firstChar;
			shift_count = 12;
		}
		else if (c <= 0x03FFFFFFUL) {
			unsigned char firstChar = 0xf8 | (unsigned char)(c >> 24);
			buf += firstChar;
			shift_count = 18;
		}
		else if (c <= 0x7FFFFFFFUL) {
			unsigned char firstChar = 0xfc | (unsigned char)(c >> 30);
			buf += firstChar;
			shift_count = 24;
		}
		else {
			assert(false);
		}
		while (shift_count >= 0) {
			unsigned char fc = 0x80 | (unsigned char)((c >> shift_count) & 0x3f);
			buf += fc;
			shift_count -= 6;
		}
	}

	return buf;
}

std:: string toUTF8String(const std:: basic_string<MYWCHAR_T> &str)
{
	std:: string buf;
	for (size_t i = 0; i < str.length(); ++i) {
		int shift_count = -1;
		MYWCHAR_T c = str[i];
		if (c != 0x00 && c <= 0x7f) {
			buf += (char)(unsigned char)c;
		}
		else if (c <= 0x07ff) {
			unsigned char firstChar = 0xc0 | (unsigned char)(c >> 6);
			buf += firstChar;
			shift_count = 0;
		}
		else if (c <= 0xffff) {
			unsigned char firstChar = 0xe0 | (unsigned char)(c >> 12);
			buf += firstChar;
			shift_count = 6;
		}
		else if (c <= 0x001FFFFFUL) {
			unsigned char firstChar = 0xf0 | (unsigned char)(c >> 18);
			buf += firstChar;
			shift_count = 12;
		}
		else if (c <= 0x03FFFFFFUL) {
			unsigned char firstChar = 0xf8 | (unsigned char)(c >> 24);
			buf += firstChar;
			shift_count = 18;
		}
		else if (c <= 0x7FFFFFFFUL) {
			unsigned char firstChar = 0xfc | (unsigned char)(c >> 30);
			buf += firstChar;
			shift_count = 24;
		}
		else {
			assert(false);
		}
		while (shift_count >= 0) {
			unsigned char fc = 0x80 | (unsigned char)((c >> shift_count) & 0x3f);
			buf += fc;
			shift_count -= 6;
		}
	}
	return buf;
}

std:: string toUTF8String(const std:: vector<MYWCHAR_T> &str)
{
	if (! str.empty()) {
		return toUTF8String(&str[0], str.size());
	}
	return std::string();
}

void addUTF8String(std:: string *pDst, MYWCHAR_T c)
{
	int shift_count = -1;
	std:: string &buf = *pDst;
	if (c != 0x00 && c <= 0x7f) {
		buf += (char)(unsigned char)c;
	}
	else if (c <= 0x07ff) {
		unsigned char firstChar = 0xc0 | (unsigned char)(c >> 6);
		buf += firstChar;
		shift_count = 0;
	}
	else if (c <= 0xffff) {
		unsigned char firstChar = 0xe0 | (unsigned char)(c >> 12);
		buf += firstChar;
		shift_count = 6;
	}
	else if (c <= 0x001FFFFFUL) {
		unsigned char firstChar = 0xf0 | (unsigned char)(c >> 18);
		buf += firstChar;
		shift_count = 12;
	}
	else if (c <= 0x03FFFFFFUL) {
		unsigned char firstChar = 0xf8 | (unsigned char)(c >> 24);
		buf += firstChar;
		shift_count = 18;
	}
	else if (c <= 0x7FFFFFFFUL) {
		unsigned char firstChar = 0xfc | (unsigned char)(c >> 30);
		buf += firstChar;
		shift_count = 24;
	}
	else {
		assert(false);
	}
	while (shift_count >= 0) {
		unsigned char fc = 0x80 | (unsigned char)((c >> shift_count) & 0x3f);
		buf += fc;
		shift_count -= 6;
	}
}

size_t toWString(MYWCHAR_T *buf, const char *str, size_t strLength)
{
	size_t j = 0;
	size_t i = 0;
	while (i < strLength) {
		MYWCHAR_T ch = firstCharUTF8String(str + i);
		buf[j++] = ch;
		size_t nextI = nextCharUTF8String(str, strLength, i);
		if (nextI == i) {
			return i; // error: invalid string
		}
		assert(nextI > i);
		i = nextI;
	}
	return j;
}

std:: basic_string<MYWCHAR_T> toWString(const std:: string &str)
{
	const int maxStackAllocSize = 512 * 1024;
	size_t bufSize = str.length();
	DECL_ALLOCA_ARRAY(stackBuf, MYWCHAR_T, bufSize <= maxStackAllocSize ? bufSize : 0);

	MYWCHAR_T *buf = stackBuf.size() > 0 ? stackBuf.c_array() : (new MYWCHAR_T[bufSize]);
	size_t bufLen = toWString(buf, str.data(), str.length());
	assert(bufLen <= str.length());
	std:: basic_string<MYWCHAR_T> r(buf, bufLen);

	if (buf != stackBuf.data()) {
		delete [] buf;
	}
	return r;
}

std:: vector<MYWCHAR_T> toWStringV(const std:: string &str)
{
	if (! str.empty()) {
		size_t bufSize = str.length();
		std:: vector<MYWCHAR_T> buf;
		buf.resize(bufSize + 1);
		size_t bufLen = toWString(&buf[0], str.data(), str.length());
		assert(bufLen <= str.length());
		buf.resize(bufLen);
		return buf;
	}
	return std::vector<MYWCHAR_T>();
}

void toWStringV(std:: vector<MYWCHAR_T> *pBuf, const std:: string &str)
{
	std:: vector<MYWCHAR_T> &buf = *pBuf;
	if (! str.empty()) {
		size_t bufSize = str.length();
		buf.resize(bufSize + 1);
		size_t bufLen = toWString(&buf[0], str.data(), str.length());
		assert(bufLen <= str.length());
		buf.resize(bufLen);
		return;
	}
	buf.resize(0);
}

void toWStringV(std:: vector<MYWCHAR_T> *pBuf, const char *str, size_t strLength)
{
	std:: vector<MYWCHAR_T> &buf = *pBuf;
	if (strLength != 0) {
		size_t bufSize = strLength;
		buf.resize(bufSize + 1);
		size_t bufLen = toWString(&buf[0], str, strLength);
		assert(bufLen <= strLength);
		buf.resize(bufLen);
		return;
	}
	buf.resize(0);
}


int compareWStringUTF8(const MYWCHAR_T *pWStr, size_t wStrLength, const char *pUtf8Str, size_t utf8StrLength) // returns -1, 0, 1
{
	size_t wi = 0;
	size_t ui = 0;
	while (wi < wStrLength && ui < utf8StrLength) {
		MYWCHAR_T wch = pWStr[wi];
		++wi;
		MYWCHAR_T uch = firstCharUTF8String(pUtf8Str + ui);
		size_t nextI = nextCharUTF8String(pUtf8Str, utf8StrLength, ui);
		if (nextI == ui) {
			return 0; // error
		}
		assert(nextI > ui);
		ui = nextI;
		if (wch != uch) {
			return wch < uch ? -1:1;
		}
	}
	if (wi == wStrLength) {
		if (ui == utf8StrLength) {
			return 0; // same length
		}
		else {
			return -1; // wStr is shorter than utf8Str
		}
	}
	else {
		return 1; // wStr is longer than utf8Str
	}
}

#endif // WSTRING_CONVERSION_SUPPORT

MYWCHAR_T firstCharUTF8String(const char *str)
{
	unsigned char ch = (unsigned char)(*str);
	
	MYWCHAR_T r = 0;
	int followingBytes = 0;
	if ((ch & 0x80) == 0) {
		assert(ch != 0);
		return ch;
	}
	else if ((ch & 0xfe) == 0xfc) {
		r |= (*str) & 0x01;
		followingBytes = 5;
	}
	else if ((ch & 0xfc) == 0xf8) {
		r |= (*str) & 0x03;
		followingBytes = 4;
	}
	else if ((ch & 0xf8) == 0xf0) {
		r |= (*str) & 0x07;
		followingBytes = 3;
	}
	else if ((ch & 0xf0) == 0xe0) {
		r |= (*str) & 0x0f;
		followingBytes = 2;
	}
	else if ((ch & 0xe0) == 0xc0) {
		r |= (*str) & 0x1f;
		followingBytes = 1;
	}
	else {
		return 0; // error: invalid
	}
	for (int i = 1; i <= followingBytes; ++i) {
		r = (r << 6) | ((*(str + i)) & 0x3f);
	}
	return r;
}

MYWCHAR_T firstCharUTF8String(const std:: string &str)
{
	return firstCharUTF8String(str.c_str());
}

size_t nextCharUTF8String(const char *str, size_t strLength, size_t index)
{
	if (! (index < strLength)) {
		return strLength;
	}

	unsigned char ch = (unsigned char)str[index];
	if ((ch & 0x80) == 0) {
		return index + 1;
	}
	else if ((ch & 0xfe) == 0xfc) {
		return index + 6;
	}
	else if ((ch & 0xfc) == 0xf8) {
		return index + 5;
	}
	else if ((ch & 0xf0) == 0xf0) {
		return index + 4;
	}
	else if ((ch & 0xf0) == 0xe0) {
		return index + 3;
	}
	else if ((ch & 0xe0) == 0xc0) {
		return index + 2;
	}
	else {
		assert((ch & 0xc0) == 0x80);
		size_t i = index;
		while ((ch & 0xc0) == 0x80 && i < strLength) {
			++i;
			ch = (unsigned char)str[i];
		}
		return i;
	}
}

size_t nextCharUTF8String(const std:: string &str, size_t index)
{
	if (! (index < str.length())) {
		return str.length();
	}

	unsigned char ch = (unsigned char)str[index];
	if ((ch & 0x80) == 0) {
		return index + 1;
	}
	else if ((ch & 0xfe) == 0xfc) {
		return index + 6;
	}
	else if ((ch & 0xfc) == 0xf8) {
		return index + 5;
	}
	else if ((ch & 0xf0) == 0xf0) {
		return index + 4;
	}
	else if ((ch & 0xf0) == 0xe0) {
		return index + 3;
	}
	else if ((ch & 0xe0) == 0xc0) {
		return index + 2;
	}
	else {
		assert((ch & 0xc0) == 0x80);
		size_t i = index;
		while ((ch & 0xc0) == 0x80 && i < str.length()) {
			++i;
			ch = (unsigned char)str[i];
		}
		return i;
	}
}

std:: string remapper(const char *str, size_t strLength)
{
	const int maxStackAllocSize = 512 * 1024;
	size_t bufSize = strLength;
	DECL_ALLOCA_ARRAY(stackBuf, char, strLength <= maxStackAllocSize ? strLength : 0);

	char *buf = stackBuf.size() > 0 ? stackBuf.c_array() : (new char[strLength]);
	char *bufP = buf;
	
	size_t i = 0; 
	while (i < strLength) {
		switch ((unsigned char)str[i]) {
		case 0xc2:
			if (i + 1 < strLength) {
				int ch = (unsigned char)str[i + 1];
				switch (ch) {
				case 0xa5:
					*bufP++ = (char)0x5c;
					i += 2;
					break;
				case 0xa6:
					*bufP++ = (char)0x7c;
					i += 2;
					break;
				case 0xaf:
					*bufP++ = (char)0x7e;
					i += 2;
					break;
				default:
					*bufP++ = str[i];
					*bufP++ = ch;
					i += 2;
				}
			}
			else {
				*bufP++ = str[i];
				i += 1;
			}
			break;
		case 0xe2:
			if (i + 2 < strLength) {
				std:: string s3(str + i, 3);
				if (s3 == "\xe2\x80\xb2") {
					*bufP++ = (char)0x27;
					i += 3;
				}
				else if (s3 == "\xe2\x80\xb3") {
					*bufP++ = (char)0x22;
					i += 3;
				}
				else if (s3 == "\xe2\x80\xbe") {
					*bufP++ = (char)0x7e;
					i += 3;
				}
				else if (s3 == "\xe2\x81\x84") {
					*bufP++ = (char)0x2f;
					i += 3;
				}
				else {
					*bufP++ = str[i];
					*bufP++ = str[i + 1];
					*bufP++ = str[i + 2];
					i += 3;
				}
			}
			else {
				*bufP++ = str[i];
				i += 1;
			}
			break;
		default:
			*bufP++ = str[i];
			i += 1;
			break;
		}
	}

	std:: string r(buf, bufP - buf);
	if (buf != stackBuf.data()) {
		delete [] buf;
	}
	return r;
}

std:: string remapper(const std:: string &str)
{
	return remapper(str.data(), str.length());
}

const std:: string UTF8BOM("\xef\xbb\xbf");

