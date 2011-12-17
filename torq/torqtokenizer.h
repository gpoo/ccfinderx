#if ! defined TORQTOKENIER_H
#define TORQTOKENIER_H

#include <vector>
#include <cstdio> // import strtoul

#include <boost/cstdint.hpp>

#include "torqcommon.h"

struct TOKEN {
public:
	enum CLASSIFICATION {
		NUL = 0,
		IDENTIFIER,
		STRING,
		LPAREN, // (
		RPAREN, // )
		LBRACE, // {
		RBRACE, // }
		COMMA, // ,
		SEMICOLON, // ;
		OPE_EQUAL, // =
		OPE_MATCH_EQ, // match=
		OPE_MATCH, // match
		OPE_SCAN_EQ, // scan=
		OPE_SCAN, // scan
		OPE_STAR, // *
		OPE_PLUS, // +
		OPE_QUES, // ?
		OPE_OR, // |
		OPE_RECURSE, // ^
		OPE_PACK, // <-
		XCEP, // xcep
		PREQ, // preq
		INSERT, // insert
		USR_BIN_TORQ, // #!/usr/bin/torq
	};
public:
	CLASSIFICATION classification;
	boost::int32_t beginPos;
	boost::int32_t endPos;
public:
	TOKEN()
		: classification(NUL), beginPos(-1), endPos(-1)
	{
	}
	TOKEN(const TOKEN &right)
		: classification(right.classification), beginPos(right.beginPos), endPos(right.endPos)
	{
	}
public:
	void swap(TOKEN &right)
	{
		std:: swap(this->classification, right.classification);
		std:: swap(this->beginPos, right.beginPos);
		std:: swap(this->endPos, right.endPos);
	}
public:
	bool operator==(const TOKEN &right) const
	{
		return this->classification == right.classification
				&& this->beginPos == right.beginPos
				&& this->endPos == right.endPos;
	}
};

class TorqTokenizer {
private:
	boost::int32_t/* end pos */ eat_space(const std:: vector<MYWCHAR_T> &str, boost::int32_t pos)
	{
		boost::int32_t si = pos;
		while (si < str.size()) {
			MYWCHAR_T ch = str[si];
			switch (ch) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				++si;
				break;
			default:
				{
					const std:: string commentBegin = "//";
					const std:: string commentEnd = "\n";
					if (common::substrEqual(str, si, commentBegin)) {
						boost::int32_t q = common::substrFind(str, si + commentBegin.length(), commentEnd);
						if (q == -1) {
							return si;
						}
						si = q + commentEnd.length();
					}
					else {
						const std:: string optionBegin = "--";
						const std:: string optionEnd = "\n";
						if (common::substrEqual(str, si, optionBegin)) {
							boost::int32_t q = common::substrFind(str, si + optionBegin.length(), optionEnd);
							if (q == -1) {
								return si;
							}
							si = q + optionEnd.length();
						}
						else {
							return si;
						}
					}
					break;
				}
			}
		}
		return si;
	}

	boost::int32_t/* end pos */ eat_token(TOKEN *pToken, const std:: vector<MYWCHAR_T> &str, boost::int32_t pos)
	{
		if (! (pos < str.size())) {
			return -1;
		}

		// reserved words
		struct TOKEN_LIST_ITEM { std:: string str; TOKEN::CLASSIFICATION cls; };
		static const TOKEN_LIST_ITEM TOKEN_LIST[] = {
			{ "match=", TOKEN::OPE_MATCH_EQ },
			{ "match", TOKEN::OPE_MATCH },
			{ "scan=", TOKEN::OPE_SCAN_EQ },
			{ "scan", TOKEN::OPE_SCAN },
			{ "<-", TOKEN::OPE_PACK },
			{ "(", TOKEN::LPAREN },
			{ ")", TOKEN::RPAREN },
			{ "{", TOKEN::LBRACE },
			{ "}", TOKEN::RBRACE },
			{ ",", TOKEN::COMMA },
			{ ";", TOKEN::SEMICOLON },
			{ "=", TOKEN::OPE_EQUAL },
			{ "*", TOKEN::OPE_STAR },
			{ "+", TOKEN::OPE_PLUS },
			{ "?", TOKEN::OPE_QUES },
			{ "|", TOKEN::OPE_OR },
			{ "^", TOKEN::OPE_RECURSE },
			{ "xcep", TOKEN::XCEP },
			{ "preq", TOKEN::PREQ },
			{ "insert", TOKEN::INSERT },
			{ "#!/usr/bin/torq", TOKEN::USR_BIN_TORQ },
			{ "", TOKEN::NUL }
		};

		//std:: string debugStr = common::HTMLEncoder().encode(str, pos, str.size() - pos);
		for (size_t ti = 0; TOKEN_LIST[ti].cls != TOKEN::NUL; ++ti) {
			const TOKEN_LIST_ITEM &t = TOKEN_LIST[ti];
			if (common::substrEqual(str, pos, t.str)) {
				boost::int32_t endPos = pos + t.str.length();
				TOKEN token;
				token.classification = t.cls;
				token.beginPos = pos;
				token.endPos = endPos;
				(*pToken).swap(token);
				return endPos;
			}
		}

		// identifier
		{
			boost::int32_t si = pos;

			MYWCHAR_T ch = str[si];
			if ('a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_') {
				++si;
				while (si < str.size() 
						&& ('a' <= (ch = str[si]) && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_' || '0' <= ch && ch <= '9')) {
					++si;
				}
				TOKEN token;
				token.classification = TOKEN::IDENTIFIER;
				token.beginPos = pos;
				token.endPos = si;
				(*pToken).swap(token);
				return si;
			}
		}
		
		// string literal
		{
			boost::int32_t si = pos;

			if (str[si] == '\"') {
				++si;
				while (si < str.size() && str[si] != '\"') {
					MYWCHAR_T ch = str[si];
					if (str[si] < 0x20) { // is a control char
						return -1;
					}
					++si;
				}
				if (! (si < str.size())) {
					return -1;
				}
				assert(str[si] == '\"');
				++si;

				TOKEN token;
				token.classification = TOKEN::STRING;
				token.beginPos = pos;
				token.endPos = si;
				(*pToken).swap(token);
				return si;
			}
		}

		return -1;
	}

	boost::int32_t/* error pos */ tokenize(std:: vector<TOKEN> *pTokens, const std:: vector<MYWCHAR_T> &script)
	{
		std:: vector<TOKEN> tokens;
		
		boost::int32_t si = 0;
		while (si < script.size()) {
			boost::int32_t spaceEnd = eat_space(script, si);
			si = spaceEnd;

			if (! (si < script.size())) {
				break; // while
			}
			
			TOKEN t;
			boost::int32_t tokenEnd = eat_token(&t, script, si);
			if (tokenEnd < 0) {
				return si;
			}
			tokens.push_back(t);
			si = tokenEnd;
		}

		(*pTokens).swap(tokens);
		return -1; // no error
	}
private:
	std:: vector<MYWCHAR_T> script;
	std:: vector<TOKEN> tokens;
	boost::int32_t errorPos;
public:
	void setScript(const std:: vector<MYWCHAR_T> &script_)
	{
		script = script_;
		tokens.clear();
	}
	bool tokenize()
	{
		errorPos = -1; // clear
		errorPos = tokenize(&tokens, script);
		if (errorPos >= 0) {
			return false; // fail
		}
		return true;
	}
	boost::int32_t getErrorPos() const
	{
		return errorPos;
	}
	std:: vector<TOKEN> getTokens() const
	{
		assert(errorPos < 0);
		return tokens;
	}
	std:: vector<MYWCHAR_T> getTokenString(boost::int32_t tokenIndex) const
	{
		assert(0 <= tokenIndex && tokenIndex < tokens.size());
		const TOKEN &t = tokens[tokenIndex];
		return getTokenString(t);
	}
	std:: vector<MYWCHAR_T> getTokenString(const TOKEN &t) const
	{
		assert(0 <= t.beginPos && t.beginPos <= t.endPos && t.endPos <= script.size());
		std:: vector<MYWCHAR_T> subscr;
		subscr.insert(subscr.end(), script.begin() + t.beginPos, script.begin() + t.endPos);
		return subscr;
	}
};


#endif // TORQTOKENIER_H
