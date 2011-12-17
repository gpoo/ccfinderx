#if ! defined Base64Encoder_H
#define Base64Encoder_H

#include <string>
#include <vector>

class Base64Encoder {
public:
	static void encode(std::string *pOutput, const char *buffer, size_t buffer_size);
	static bool decode(std::vector<char> *pBuffer, const std::string &input);
};

#endif // Base64Encoder_H

