#if ! defined UTF8SUPPORT_HEADER
#define UTF8SUPPORT_HEADER

#include <string>
#include <vector>
#include <cassert>

#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include <unicode/schriter.h>

typedef UChar32 MYWCHAR_T; // 4byte

size_t countCharUTF8String(const char *str, size_t strLength);
size_t countCharUTF8String(const std:: string &str);

#if defined WSTRING_CONVERSION_SUPPORT

size_t toUTF8String(char *buf, const MYWCHAR_T *str, size_t strLength);
std:: string toUTF8String(const MYWCHAR_T *str, size_t strLength);
std:: string toUTF8String(const std:: basic_string<MYWCHAR_T> &str);
std:: string toUTF8String(const std:: vector<MYWCHAR_T> &str);
void addUTF8String(std:: string *pDst, MYWCHAR_T c);

size_t toWString(MYWCHAR_T *buf, const char *str, size_t strLength);
std:: basic_string<MYWCHAR_T> toWString(const std:: string &str);
std:: vector<MYWCHAR_T> toWStringV(const std:: string &str);
void toWStringV(std:: vector<MYWCHAR_T> *pBuf, const std:: string &str);
void toWStringV(std:: vector<MYWCHAR_T> *pBuf, const char *str, size_t strLength);

int compareWStringUTF8(const MYWCHAR_T *pWStr, size_t wStrLength, const char *pUtf8Str, size_t utf8StrLength); // returns -1, 0, 1
inline int compareWStringUTF8(const std::vector<MYWCHAR_T> &wStr, const std::string &utf8Str) // returns -1, 0, 1
{
	return compareWStringUTF8(&wStr[0], wStr.size(), utf8Str.data(), utf8Str.length());
}

#endif

size_t nextCharUTF8String(const char *str, size_t strLength, size_t index);
size_t nextCharUTF8String(const std:: string &str, size_t index);

MYWCHAR_T firstCharUTF8String(const char *str);
MYWCHAR_T firstCharUTF8String(const std:: string &str);

std:: string remapper(const std:: string &pstr);
std:: string remapper(const char *str, size_t strLength);

extern const std:: string UTF8BOM;

inline std:: string toLower(const std:: string &str)
{
	std:: string r;
	r.reserve(str.length());
	for (size_t i = 0; i < str.length(); ++i) {
		int ch = str[i];
		if ('A' <= ch && ch <= 'Z') {
			ch = ch - 'A' + 'a';
		}
		r += (char)ch;
	}
	return r;
}

#if defined CODE_CONVERSION_SUPPORT

class Decoder {
private:
	UConverter* pCnv;
	std:: string encodingName;
public:
	Decoder()
		: pCnv(NULL), encodingName()
	{
		UErrorCode error = U_ZERO_ERROR;
		pCnv = ucnv_open(NULL, &error);
	}
	Decoder(const Decoder &rhs)
		: pCnv(NULL), encodingName()
	{
		setEncoding(rhs.encodingName);
	}
	~Decoder() 
	{
		if (pCnv != NULL) {
			ucnv_close(pCnv);
			pCnv = NULL;
		}
	}
	Decoder &operator=(const Decoder &right)
	{
		if (&right == this) {
			return *this;
		}

		if (pCnv != NULL) {
			ucnv_close(pCnv);
			pCnv = NULL;
		}
		setEncoding(right.encodingName);

		return *this;
	}
	bool setEncoding(const std:: string &encodingName_)
	{
		if (pCnv != NULL) {
			ucnv_close(pCnv);
			pCnv = NULL;
		}

		encodingName = encodingName_;

		UErrorCode error = U_ZERO_ERROR;
		if (encodingName.empty() || encodingName == "char") {
			pCnv = ucnv_open(NULL, &error);
		}
		else {
			pCnv = ucnv_open(encodingName.c_str(), &error);
		}
		if (! U_SUCCESS(error)) {
			return false;
		}

		return true;
	}
	std:: string getEncoding() const
	{
		return encodingName;
	}
	std:: vector<std:: string> getAvailableEncodings() const
	{
		std:: vector<std:: string> names;
		size_t count = ucnv_countAvailable();
		for (size_t i = 0; i < count; ++i) {
			std:: string name = ucnv_getAvailableName(i);
			
			UErrorCode error = U_ZERO_ERROR;
			size_t aliasCount = ucnv_countAliases(name.c_str(), &error);
			if (U_SUCCESS(error)) {
				std:: vector<const char *> aliasData;
				aliasData.resize(aliasCount);
				ucnv_getAliases(name.c_str(), &aliasData[0], &error);
				if (U_SUCCESS(error)) {
					for (size_t j = 0; j < aliasCount; ++j) {
						names.push_back(std::string(aliasData[j]));
					}
				}
			}
		}
		return names;
	}
	inline std:: vector<MYWCHAR_T> decode(const std:: string &str)
	{
		return decode(str.data(), str.data() + str.length());
	}
	std:: vector<MYWCHAR_T> decode(const char *begin, const char *end)
	{
		assert(end >= begin);

		if (pCnv == NULL) {
			return std::vector<MYWCHAR_T>();
		}

		UErrorCode error = U_ZERO_ERROR;
		UnicodeString ustr(begin, end - begin, pCnv, error);
		if (! U_SUCCESS(error)) {
			return std::vector<MYWCHAR_T>();
		}
		
		size_t count = ustr.countChar32();
		std:: vector<MYWCHAR_T> buf;
		buf.reserve(count);

		StringCharacterIterator it(ustr);
		for (UChar uc = it.first32(); uc != it.DONE; uc = it.next32()) {
			if (uc != 0xfeff/* bom */) {
				buf.push_back(uc);
			}
		}

		return buf;
	}
	inline void decode(std:: vector<MYWCHAR_T> *pResult, const std:: string &str)
	{
		decode(pResult, str.data(), str.data() + str.length());
	}
	void decode(std:: vector<MYWCHAR_T> *pResult, const char *begin, const char *end)
	{
		assert(end >= begin);

		if (pCnv == NULL) {
			(*pResult).clear();
			return;
		}

		UErrorCode error = U_ZERO_ERROR;
		UnicodeString ustr(begin, end - begin, pCnv, error);
		if (! U_SUCCESS(error)) {
			(*pResult).clear();
			return;
		}
		
		size_t count = ustr.countChar32();
		(*pResult).reserve(count);

		(*pResult).clear();
		StringCharacterIterator it(ustr);
		for (UChar uc = it.first32(); uc != it.DONE; uc = it.next32()) {
			if (uc != 0xfeff/* bom */) {
				(*pResult).push_back(uc);
			}
		}
	}
	std:: string encode(const MYWCHAR_T *pStr, size_t strLength)
	{
		UnicodeString ustr(strLength * 2, (UChar32)' ', 0); // reserve (strLength * 2) * 16bits

		for (size_t i = 0; i < strLength; ++i) {
			ustr.append((UChar32)*(pStr + i));
		}

		UErrorCode error;
		size_t requiredBufferSize = ustr.extract(NULL, 0, pCnv, error = U_ZERO_ERROR);
		std:: vector<char> buffer(requiredBufferSize);
		size_t length = ustr.extract(&buffer[0], buffer.size(), pCnv, error = U_ZERO_ERROR);
		if (! U_SUCCESS(error)) {
			return std::string();
		}
		assert(length == requiredBufferSize);

		return std:: string(&buffer[0], length);
	}
	inline std:: string encode(const std:: vector<MYWCHAR_T> &data)
	{
		if (! data.empty()) {
			return encode(&data[0], data.size());
		}
		return std::string();
	}
//private:
//	void decode_i(std:: vector<MYWCHAR_T> *pResult, const char *begin, const char *end, iconv_t convtr)
//	{
//		(*pResult).clear();
//		size_t strLength = end - begin;
//
//		std:: vector<MYWCHAR_T> &buf = *pResult;
//		if (reinterpret_cast<int>(convtr) == -1) {
//			buf.resize(strLength);
//			for (size_t i = 0; i < strLength; ++i) {
//				buf.push_back(*(begin + i));
//			}
//		}
//
//		if (buf.size() < strLength) {
//			buf.resize(strLength);
//		}
//
//		size_t bufRest = buf.size() * sizeof(MYWCHAR_T);
//		size_t strRest = strLength;
//		const char *strP = begin;
//		while (strP < end) {
//			// find the delimiter of line
//			const char *eol = strP;
//			size_t delimLen = 0;
//			while (eol < end && ! (*eol == '\n' || *eol == '\r')) {
//				++eol;
//			}
//			if (eol < end) {
//				if (*eol == '\r') {
//					if (eol + 1 < end && *(eol + 1) == '\n') {
//						delimLen = 2;
//					}
//					else {
//						delimLen = 1;
//					}
//				}
//				else if (*eol == '\n') {
//					delimLen = 1;
//				}
//				else {
//					assert(false);
//				}
//			}
//
//			// here, range strP to eol is a line
//
//			// copy the line
//			size_t strRestToEol = eol - strP;
//			while (strRestToEol > 0) {
//				char *bufP = reinterpret_cast<char*>(&buf[0]) + buf.size() * sizeof(MYWCHAR_T) - bufRest;
//#if ! defined DEBIAN_PATCH
//				size_t r = iconv(convtr, &strP, &strRestToEol, &bufP, &bufRest);
//#else
//				size_t r = iconv(convtr, (char**)&strP, &strRestToEol, &bufP, &bufRest);
//#endif
//				if (strRestToEol == 0) {
//					break;
//				}
//				if (bufRest > 0) {
//					// the byte is illegal charactor
//					buf[buf.size() - bufRest / sizeof(MYWCHAR_T)] = *strP++;
//					bufRest -= sizeof(MYWCHAR_T);
//					--strRestToEol;
//				}
//				else {
//					size_t addition = buf.size();
//					buf.resize(buf.size() + addition);
//					bufRest += addition * sizeof(MYWCHAR_T);
//				}
//			}
//			assert(strRestToEol == 0);
//
//			// copy the delimiter
//			strP = eol;
//			if (bufRest < delimLen) {
//				size_t addition = delimLen;
//				buf.resize(buf.size() + addition);
//				bufRest += addition * sizeof(MYWCHAR_T);
//			}
//			{
//				char *bufP = reinterpret_cast<char*>(&buf[0]) + buf.size() * sizeof(MYWCHAR_T) - bufRest;
//				for (size_t i = 0; i < delimLen; ++i) {
//					buf[buf.size() - bufRest / sizeof(MYWCHAR_T)] = *strP++;
//					bufRest -= sizeof(MYWCHAR_T);
//				}
//			}
//		}
//		buf.resize(buf.size() - bufRest / sizeof(MYWCHAR_T));
//	}
};

#endif // CODE_CONVERSION_SUPPORT

#endif // UT8SUPPORT_HEADDER

