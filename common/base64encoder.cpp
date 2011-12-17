#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

#include <boost/cstdint.hpp>

#include "base64encoder.h"

namespace {

void encode_i(std::string *pOutput, const char buf3[3])
{
	static const char table[64 + 1] = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	boost::uint32_t bits = 0;
	for (size_t i = 0; i < 3; ++i) {
		boost::uint32_t ch = buf3[i];
		bits = (bits << 8) | ch;
	}
	for (size_t i = 0; i < 4; ++i) {
		boost::uint32_t slice = (bits >> ((4 - 1 - i) * 6)) & 0x3f;
		assert(0 <= slice && slice < 64);
		(*pOutput) += table[slice];
	}
}

void decode_i(std::vector<char> *pBuffer, boost::uint32_t bits)
{
	for (size_t i = 0; i < 3; ++i) {
		boost::uint32_t slice = (bits >> ((3 - 1 - i) * 8)) & 0xff;
		assert(0 <= slice && slice < 0x100);
		(*pBuffer).push_back((char)slice);
	}
}

}; // namespace

void Base64Encoder::encode(std::string *pOutput, const char *buffer, size_t buffer_size)
{
	(*pOutput).reserve((*pOutput).size() + buffer_size * 4 + (3 - 1) / 3);

	size_t i;
	for (i = 0; i + 3 <= buffer_size; i += 3) {
		encode_i(pOutput, buffer + i);
	}
	size_t remains = buffer_size - i;
	switch (remains) {
	case 0:
		break;
	case 1:
		{
			char buf3[3] = { buffer[i], 0, 0 };
			encode_i(pOutput, buf3);
			(*pOutput)[(*pOutput).length() - 2] = '=';
			(*pOutput)[(*pOutput).length() - 1] = '=';
			break;
		}
		break;
	case 2:
		{
			char buf3[3] = { buffer[i], buffer[i + 1], 0 };
			encode_i(pOutput, buf3);
			(*pOutput)[(*pOutput).length() - 1] = '=';
		}
		break;
	default:
		assert(false);
	}
}

bool Base64Encoder::decode(std::vector<char> *pBuffer, const std::string &input)
{
	static const char table[64 + 1] = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	boost::uint32_t bits = 0;
	size_t validChars = 0;
	size_t extraChars = 0;
	for (size_t i = 0; i < input.size(); ++i) {
		if (validChars == 4) {
			decode_i(pBuffer, bits);
			validChars = 0;
		}
		const char *p = std::find(&table[0], &table[64], input[i]);
		size_t pos = p - &table[0];
		if (pos < 64) {
			bits = (bits << 6) | (boost::uint32_t)(pos);
			++validChars;
		}
		else if (input[i] == '=') {
			bits = (bits << 6) | (boost::uint32_t)(0);
			++validChars;
			++extraChars;
		}
		else {
			// simply neglect the char.
		}
	}
	if (validChars == 4) {
		decode_i(pBuffer, bits);
		validChars = 0;
	}
	if (validChars != 0) {
		return false; // error, invalid string
	}
	if (extraChars > 2) {
		return false; // error, invalid string
	}
	for (size_t i = 0; i < extraChars; ++i) {
		(*pBuffer).pop_back();
	}
	return true;
}

