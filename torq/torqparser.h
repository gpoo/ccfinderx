#if ! defined TORQPARSER_H
#define TORQPARSER_H

#include <vector>
#include <cassert>
#include <iostream>

#include <boost/cstdint.hpp>
//#include "../common/unportable.h"

#include "torqcommon.h"
#include "torqtokenizer.h"

class TorqParser {
public:
	class Callbacks {
	public:
		virtual bool enterStatements(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectStatements(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptStatements(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptStatement(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterAssignStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectAssignStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptAssignStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
		{
			return true; // success
		}
		virtual bool enterScanEqStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectScanEqStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptScanEqStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
		{
			return true; // success
		}
		virtual bool enterMatchEqStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectMatchEqStatement(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptMatchEqStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
		{
			return true; // success
		}
		virtual bool enterExpression(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectExpression(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptExpression(boost::int32_t begin, boost::int32_t end, const TOKEN &var)
		{
			return true; // success
		}
		virtual bool enterPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterPackPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectPackPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptPackPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
		{
			return true; // success
		}
		virtual bool enterMatchPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectMatchPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptMatchPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
		{
			return true; // success
		}
		virtual bool enterScanPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectScanPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptScanPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
		{
			return true; // success
		}
		virtual bool enterOrPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectOrPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptOrPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterSequencePattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectSequencePattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptSequencePattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterRepeatPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectRepeatPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptRepeatPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
		{
			return true; // success
		}
		virtual bool acceptRepeatPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterAtomPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectAtomPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptAtomPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterParenPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectParenPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptParenPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterXcepPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectXcepPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptXcepPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterPreqPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectPreqPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptPreqPattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
		virtual bool enterLiteralPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectLiteralPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptLiteralPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &literal)
		{
			return true; // success
		}
		virtual bool enterGeneratedTokenPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectGeneratedTokenPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptGeneratedTokenPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &generatedToken)
		{
			return true; // success
		}
		virtual bool enterInsertPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectInsertPattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptInsertPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &generatedToken)
		{
			return true; // success
		}
		virtual bool enterRecursePattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool rejectRecursePattern(boost::int32_t begin)
		{
			return true; // success
		}
		virtual bool acceptRecursePattern(boost::int32_t begin, boost::int32_t end)
		{
			return true; // success
		}
	};
public:
	enum ParseErrorCode {
		NUL = 0,
		ACTION_ERROR,
		UNEXPECTED_EOF,
	};
private:
	std:: vector<TOKEN> tokens;
	boost::int32_t errorPos;
	ParseErrorCode errorCode;
	TorqParser::Callbacks *pCallback;
public:
	virtual ~TorqParser()
	{
	}
public:
	TorqParser()
		: tokens(), errorPos(-1), errorCode(NUL), pCallback(NULL)
	{
	}
	void setTokens(const std:: vector<TOKEN> &tokens_)
	{
		tokens = tokens_;
	}
	bool parse(TorqParser::Callbacks *pCallback_)
	{
		TorqParser::Callbacks dummyCallback;
		if (pCallback_ != NULL) {
			pCallback = pCallback_;
		}
		else {
			pCallback = &dummyCallback;
		}

		errorPos = -1; // clear
		boost::int32_t pos = parseUsrBinTorq(0);
		pos = parseStatements(pos);
		if (errorPos >= 0) {
			if (pCallback == &dummyCallback) {
				pCallback = NULL;
			}
			return false; // error
		}
		if (pos != tokens.size()) {
			errorPos = pos;
			errorCode = UNEXPECTED_EOF;
			if (pCallback == &dummyCallback) {
				pCallback = NULL;
			}
			return false; // error
		}
		if (pCallback == &dummyCallback) {
			pCallback = NULL;
		}
		return true;
	}
	boost::int32_t getErrorPos() const
	{
		return errorPos;
	}
public:
	boost::int32_t parseUsrBinTorq(boost::int32_t pos0)
	{
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::USR_BIN_TORQ) {
			return pos + 1;
		}
		else {
			return pos;
		}
	}
	boost::int32_t parseStatements(boost::int32_t pos0)
	{
		if (! pCallback->enterStatements(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		while (true) {
			boost::int32_t p = parseStatement(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
			}
			else {
				break; // while
			}
		}
		if (! pCallback->acceptStatements(pos0, pos)) {
			errorPos = pos0;
			return pos0; // error
		}
		return pos; // match
	}
	boost::int32_t parseStatement(boost::int32_t pos0)
	{
		if (! pCallback->enterStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		boost::int32_t p = parseAssignStatement(pos);
		if (errorPos >= 0) {
			return pos; // error
		}
		if (p >= 0) {
			pos = p;
			pCallback->acceptStatement(pos0, pos);
			return pos; // match
		}
		else {
			p = parseScanEqStatement(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
				pCallback->acceptStatement(pos0, pos);
				return pos; // match
			}
			else {
				p = parseMatchEqStatement(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
					if (! pCallback->acceptStatement(pos0, pos)) {
						errorPos = pos0;
						return pos0; // error
					}
					return pos; // match
				}
				else {
					if (! pCallback->rejectStatement(pos0)) {
						errorPos = pos0;
						return pos0; // error
					}
					return -1; // unmatch
				}
			}
		}
	}
	boost::int32_t parseAssignStatement(boost::int32_t pos0)
	{
		if (! pCallback->enterAssignStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
			const TOKEN &leftVar = tokens[pos];
			++pos;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_EQUAL) {
				++pos;
				boost::int32_t p = parseExpression(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
					if (pos < tokens.size() && tokens[pos].classification == TOKEN::SEMICOLON) {
						++pos;
						if (! pCallback->acceptAssignStatement(pos0, pos, leftVar)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
				}
			}
		}
		if (! pCallback->rejectAssignStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parseScanEqStatement(boost::int32_t pos0)
	{
		if (! pCallback->enterScanEqStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		TOKEN t;
		if (pos < tokens.size() && (t = tokens[pos]).classification == TOKEN::IDENTIFIER) {
			const TOKEN &leftVar = tokens[pos];
			++pos;
			if (pos < tokens.size() && (t = tokens[pos]).classification == TOKEN::OPE_SCAN_EQ) {
				++pos;
				boost::int32_t p = parsePattern(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
					if (pos < tokens.size() && (t = tokens[pos]).classification == TOKEN::SEMICOLON) {
						++pos;
						if (! pCallback->acceptScanEqStatement(pos0, pos, leftVar)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
				}
			}
		}
		if (! pCallback->rejectScanEqStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseMatchEqStatement(boost::int32_t pos0)
	{
		if (! pCallback->enterMatchEqStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
			const TOKEN &leftVar = tokens[pos];
			++pos;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_MATCH_EQ) {
				++pos;
				boost::int32_t p = parsePattern(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
					if (pos < tokens.size() && tokens[pos].classification == TOKEN::SEMICOLON) {
						++pos;
						if (! pCallback->acceptMatchEqStatement(pos0, pos, leftVar)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
				}
			}
		}
		if (! pCallback->rejectMatchEqStatement(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseExpression(boost::int32_t pos0)
	{
		if (! pCallback->enterExpression(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
			const TOKEN &val = tokens[pos];
			++pos;
			if (! pCallback->acceptExpression(pos0, pos, val)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectExpression(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parsePattern(boost::int32_t pos0)
	{
		if (! pCallback->enterPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		boost::int32_t p = parsePackPattern(pos);
		if (errorPos >= 0) {
			return pos; // error
		}
		if (p < 0) {
			p = parseMatchPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
		}
		if (p < 0) {
			p = parseScanPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
		}
		if (p < 0) {
			p = parseOrPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
		}
		if (p >= 0) {
			pos = p;
			if (! pCallback->acceptPattern(pos0, pos)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parsePackPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterPackPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		{
			boost::int32_t pos = pos0;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
				const TOKEN &generatedToken = tokens[pos];
				++pos;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_PACK) {
					++pos;
					boost::int32_t p = parseOrPattern(pos);
					if (errorPos >= 0) {
						return pos; // error
					}
					if (p >= 0) {
						pos = p;
						if (! pCallback->acceptPackPattern(pos0, pos, generatedToken)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
					else {
						errorPos = pos0;
						return pos0; // error
					}
				}
			}
		}
		if (! pCallback->rejectPackPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseMatchPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterMatchPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		{
			boost::int32_t pos = pos0;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
				const TOKEN &generatedToken = tokens[pos];
				++pos;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_MATCH) {
					++pos;
					boost::int32_t p = parseOrPattern(pos);
					if (errorPos >= 0) {
						return pos; // error
					}
					if (p >= 0) {
						pos = p;
						if (! pCallback->acceptMatchPattern(pos0, pos, generatedToken)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
					else {
						errorPos = pos0;
						return pos0; // error
					}
				}
			}
		}
		if (! pCallback->rejectMatchPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseScanPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterScanPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		{
			boost::int32_t pos = pos0;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
				const TOKEN &generatedToken = tokens[pos];
				++pos;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_SCAN) {
					++pos;
					boost::int32_t p = parseOrPattern(pos);
					if (errorPos >= 0) {
						return pos; // error
					}
					if (p >= 0) {
						pos = p;
						if (! pCallback->acceptScanPattern(pos0, pos, generatedToken)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
					else {
						errorPos = pos0;
						return pos0; // error
					}
				}
			}
		}
		if (! pCallback->rejectScanPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseOrPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterOrPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		boost::int32_t p = parseSequencePattern(pos);
		if (errorPos >= 0) {
			return pos; // error
		}
		if (p >= 0) {
			pos = p;
			while (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_OR) {
				++pos;
				boost::int32_t p = parseOrPattern(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
				}
				else {
					errorPos = pos;
					return pos; // error
				}
			}
			if (! pCallback->acceptOrPattern(pos0, pos)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectOrPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseSequencePattern(boost::int32_t pos0)
	{
		if (! pCallback->enterSequencePattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		boost::int32_t p = parseRepeatPattern(pos);
		if (errorPos >= 0) {
			return pos; // error
		}
		if (p >= 0) {
			pos = p;
			while (true) {
				boost::int32_t p = parseSequencePattern(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
				}
				else {
					break; // while
				}
			}
			if (! pCallback->acceptSequencePattern(pos0, pos)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectSequencePattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parseRepeatPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterRepeatPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		TOKEN t;
		if (pos < tokens.size() 
				&& ((t = tokens[pos]).classification == TOKEN::OPE_STAR || t.classification == TOKEN::OPE_PLUS || t.classification == TOKEN::OPE_QUES)) {
			const TOKEN &ope = tokens[pos];
			++pos;
			boost::int32_t p = parseAtomPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
				if (! pCallback->acceptRepeatPattern(pos0, pos, ope)) {
					errorPos = pos0;
					return pos0; // error
				}
				return pos; // match
			}
			else {
				errorPos = pos0;
				return pos0; // error
			}
		}
		else {
			boost::int32_t p = parseAtomPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
				if (! pCallback->acceptRepeatPattern(pos0, pos)) {
					errorPos = pos0;
					return pos0; // error
				}
				return pos; // match
			}
		}
		if (! pCallback->rejectRepeatPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseAtomPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterAtomPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		boost::int32_t p = parseParenPattern(pos);
		if (errorPos >= 0) {
			return pos; // error
		}
		if (p >= 0) {
			pos = p;
			if (! pCallback->acceptAtomPattern(pos0, pos)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		else {
			boost::int32_t p = parseXcepPattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
				if (! pCallback->acceptAtomPattern(pos0, pos)) {
					errorPos = pos0;
					return pos0; // error
				}
				return pos; // match
			}
			else {
				boost::int32_t p = parsePreqPattern(pos);
				if (errorPos >= 0) {
					return pos; // error
				}
				if (p >= 0) {
					pos = p;
					if (! pCallback->acceptAtomPattern(pos0, pos)) {
						errorPos = pos0;
						return pos0; // error
					}
					return pos; // match
				}
				else {
					boost::int32_t pos = pos0;
					boost::int32_t p = parseLiteralPattern(pos, false);
					if (errorPos >= 0) {
						return pos; // error
					}
					if (! (p >= 0)) {
						p = parseGeneratedTokenPattern(pos);
						if (errorPos >= 0) {
							return pos; // error
						}
					}
					if (p >= 0) {
						pos = p;
						if (! pCallback->acceptAtomPattern(pos0, pos)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
					else {
						boost::int32_t p = parseInsertPattern(pos);
						if (errorPos >= 0) {
							return pos; // error
						}
						if (p >= 0) {
							pos = p;
							if (! pCallback->acceptAtomPattern(pos0, pos)) {
								errorPos = pos0;
								return pos0; // error
							}
							return pos; // match
						}
						else {
							boost::int32_t p = parseRecursePattern(pos);
							if (errorPos >= 0) {
								return pos; // error
							}
							if (p >= 0) {
								pos = p;
								if (! pCallback->acceptAtomPattern(pos0, pos)) {
									errorPos = pos0;
									return pos0; // error
								}
								return pos; // match
							}
							else {
								if (! pCallback->rejectAtomPattern(pos0)) {
									errorPos = pos0;
									return pos0; // error
								}
								return -1; // umatch
							}
						}
					}
				}
			}
		}
	}
	boost::int32_t parseParenPattern(boost::int32_t pos0) 
	{
		if (! pCallback->enterParenPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::LPAREN) {
			++pos;
			boost::int32_t p = parsePattern(pos);
			if (errorPos >= 0) {
				return pos; // error
			}
			if (p >= 0) {
				pos = p;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::RPAREN) {
					++pos;
					if (! pCallback->acceptParenPattern(pos0, pos)) {
						errorPos = pos0;
						return pos0; // error
					}
					return pos; // match
				}
				else {
					errorPos = pos0;
					return pos0; // error
				}
			}
			else {
				errorPos = pos0;
				return pos0; // error
			}
		}
		if (! pCallback->rejectParenPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // umatch
	}
	boost::int32_t parseXcepPattern(boost::int32_t pos0)
	{
		return parse_xcep_or_preq_pattern(pos0, true);
	}
	boost::int32_t parsePreqPattern(boost::int32_t pos0)
	{
		return parse_xcep_or_preq_pattern(pos0, false);
	}
	boost::int32_t parse_xcep_or_preq_pattern(boost::int32_t pos0, bool is_xcep)
	{
		if (is_xcep) {
			if (! pCallback->enterXcepPattern(pos0)) {
				errorPos = pos0;
				return pos0; // error
			}
		}
		else {
			if (! pCallback->enterPreqPattern(pos0)) {
				errorPos = pos0;
				return pos0; // error
			}
		}

		boost::int32_t pos = pos0;
		if (pos < tokens.size() 
					&& (is_xcep && tokens[pos].classification == TOKEN::XCEP || ! is_xcep && tokens[pos].classification == TOKEN::PREQ)) {
			++pos;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::LPAREN) {
				++pos;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::RPAREN) {
					++pos;
					if (is_xcep) {
						if (! pCallback->acceptXcepPattern(pos0, pos)) {
							errorPos = pos0;
							return pos0; // error
						}
					}
					else {
						if (! pCallback->acceptPreqPattern(pos0, pos)) {
							errorPos = pos0;
							return pos0; // error
						}
					}
					return pos; // match
				}
				else {
					boost::int32_t p = parseLiteralPattern(pos, true);
					if (errorPos >= 0) {
						return pos; // error
					}
					if (! (p >= 0)) {
						p = parseGeneratedTokenPattern(pos);
						if (errorPos >= 0) {
							return pos; // error
						}
					}
					if (p >= 0) {
						pos = p;
						while (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_OR) {
							++pos;
							boost::int32_t p = parseLiteralPattern(pos, true);
							if (errorPos >= 0) {
								return pos; // error
							}
							if (! (p >= 0)) {
								p = parseGeneratedTokenPattern(pos);
								if (errorPos >= 0) {
									return pos; // error
								}
							}
							if (p >= 0) {
								pos = p;
							}
							else {
								errorPos = pos0;
								return pos0; // error
							}
						}
						if (pos < tokens.size() && tokens[pos].classification == TOKEN::RPAREN) {
							++pos;
							if (is_xcep) {
								if (! pCallback->acceptXcepPattern(pos0, pos)) {
									errorPos = pos0;
									return pos0; // error
								}
							}
							else {
								if (! pCallback->acceptPreqPattern(pos0, pos)) {
									errorPos = pos0;
									return pos0; // error
								}
							}
							return pos; // match
						}
						else {
							errorPos = pos0;
							return pos0; // error
						}
					}
					else {
						errorPos = pos0;
						return pos0; // error
					}
				}
			}
			else {
				errorPos = pos0;
				return pos0; // error
			}
		}
		if (is_xcep) {
			if (! pCallback->rejectXcepPattern(pos0)) {
				errorPos = pos0;
				return pos0; // error
			}
		}
		else {
			if (! pCallback->rejectPreqPattern(pos0)) {
				errorPos = pos0;
				return pos0; // error
			}
		}
		return -1; // umatch
	}
	boost::int32_t parseLiteralPattern(boost::int32_t pos0, bool insideXcep)
	{
		if (! pCallback->enterLiteralPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::STRING) {
			const TOKEN &literal = tokens[pos];
			++pos;
			if (! pCallback->acceptLiteralPattern(pos0, pos, literal)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectLiteralPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parseGeneratedTokenPattern(boost::int32_t pos0)
	{
		if (! pCallback->enterGeneratedTokenPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
			const TOKEN &generatedToken = tokens[pos];
			++pos;
			if (! pCallback->acceptGeneratedTokenPattern(pos0, pos, generatedToken)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectGeneratedTokenPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parseInsertPattern(boost::int32_t pos0)
	{
		if (! pCallback->enterInsertPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::INSERT) {
			++pos;
			if (pos < tokens.size() && tokens[pos].classification == TOKEN::LPAREN) {
				++pos;
				if (pos < tokens.size() && tokens[pos].classification == TOKEN::IDENTIFIER) {
					const TOKEN &generatedToken = tokens[pos];
					++pos;
					if (pos < tokens.size() && tokens[pos].classification == TOKEN::RPAREN) {
						++pos;
						if (! pCallback->acceptInsertPattern(pos0, pos, generatedToken)) {
							errorPos = pos0;
							return pos0; // error
						}
						return pos; // match
					}
					else {
						errorPos = pos0;
						return pos0; // error
					}
				}
				else {
					errorPos = pos0;
					return pos0; // error
				}
			}
			else {
				errorPos = pos0;
				return pos0; // error
			}
		}
		if (! pCallback->rejectInsertPattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
	boost::int32_t parseRecursePattern(boost::int32_t pos0)
	{
		if (! pCallback->enterRecursePattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		boost::int32_t pos = pos0;
		if (pos < tokens.size() && tokens[pos].classification == TOKEN::OPE_RECURSE) {
			++pos;
			if (! pCallback->acceptRecursePattern(pos0, pos)) {
				errorPos = pos0;
				return pos0; // error
			}
			return pos; // match
		}
		if (! pCallback->rejectRecursePattern(pos0)) {
			errorPos = pos0;
			return pos0; // error
		}
		return -1; // unmatch
	}
};

enum NodeClassification {
	NC_Nul = 0,
	NC_Statements,
	NC_Statement,
	NC_AssignStatement,
	NC_ScanEqStatement,
	NC_MatchEqStatement,
	NC_Expression,
	NC_Pattern,
	NC_PackPattern,
	NC_MatchPattern,
	NC_ScanPattern,
	NC_OrPattern,
	NC_SequencePattern,
	NC_RepeatPattern,
	NC_AtomPattern,
	NC_ParenPattern,
	NC_XcepPattern,
	NC_PreqPattern,
	NC_LiteralPattern,
	NC_GeneratedTokenPattern,
	NC_RecursePattern,
	NC_InsertPattern,
NC_SIZE
};

class NodeClassificationHelper {
public:
	static std:: string toString(const NodeClassification &nc)
	{
		switch (nc) {
		case NC_Nul:              return "Nul";
		case NC_Statements:       return "Statements";
		case NC_Statement:        return "Statement";
		case NC_AssignStatement:  return "AssignStatement";
		case NC_ScanEqStatement:  return "ScanEqStatement";
		case NC_MatchEqStatement: return "MatchEqStatement";
		case NC_Expression:       return "Expression";
		case NC_Pattern:          return "Pattern";
		case NC_PackPattern:      return "PackPattern";
		case NC_MatchPattern:     return "MatchPattern";
		case NC_ScanPattern:      return "ScanPattern";
		case NC_OrPattern:        return "OrPattern";
		case NC_SequencePattern:  return "SequencePattern";
		case NC_RepeatPattern:    return "RepeatPattern";
		case NC_AtomPattern:      return "AtomPattern";
		case NC_ParenPattern:     return "ParenPattern";
		case NC_XcepPattern:      return "XcepPattern";
		case NC_PreqPattern:      return "PreqPattern";
		case NC_LiteralPattern:          return "LiteralPattern";
		case NC_GeneratedTokenPattern:   return "GeneratedTokenPattern";
		case NC_RecursePattern:   return "RecursePattern";
		case NC_InsertPattern:    return "InsertPattern";
		}
		assert(false);
		return "";
	}
};

struct TRACE_ITEM {
public:
	enum Classification { Nul = 0, Enter, Reject, Accept };
public:
	Classification classification;
	NodeClassification node;
	boost::int32_t begin;
	boost::int32_t end;
	TOKEN ref;
public:
	TRACE_ITEM()
		: classification(Nul), node(NC_Nul), begin(-1), end(-1), ref()
	{
	}
	TRACE_ITEM(const TRACE_ITEM &right)
		: classification(right.classification), node(right.node), begin(right.begin), end(right.end), ref(right.ref)
	{
	}
	TRACE_ITEM(Classification classification_, NodeClassification node_, boost::int32_t begin_)
		: classification(classification_), node(node_), begin(begin_), end(-1), ref()
	{
	}
	TRACE_ITEM(Classification classification_, NodeClassification node_, boost::int32_t begin_, boost::int32_t end_)
		: classification(classification_), node(node_), begin(begin_), end(end_), ref()
	{
	}
	TRACE_ITEM(Classification classification_, NodeClassification node_, boost::int32_t begin_, boost::int32_t end_, const TOKEN &ref_)
		: classification(classification_), node(node_), begin(begin_), end(end_), ref(ref_)
	{
	}
public:
	void swap(TRACE_ITEM &right)
	{
		std:: swap(this->classification, right.classification);
		std:: swap(this->node, right.node);
		std:: swap(this->begin, right.begin);
		std:: swap(this->end, right.end);
		this->ref.swap(right.ref);
	}
};

class TorqParserTracer : public TorqParser::Callbacks {
protected:
	std:: vector<TRACE_ITEM> trace;
public:
	std:: vector<TRACE_ITEM> getTrace() const
	{
		return trace;
	}
public:
	virtual void enter(NodeClassification node, boost::int32_t begin)
	{
		TRACE_ITEM ti(TRACE_ITEM::Enter, node, begin);
		trace.push_back(ti);
	}
	virtual void reject(NodeClassification node, boost::int32_t begin)
	{
		TRACE_ITEM ti(TRACE_ITEM::Reject, node, begin);
		trace.push_back(ti);
	}
	virtual void accept(NodeClassification node, boost::int32_t begin, boost::int32_t end)
	{
		TRACE_ITEM ti(TRACE_ITEM::Accept, node, begin, end);
		trace.push_back(ti);
	}
	virtual void accept(NodeClassification node, boost::int32_t begin, boost::int32_t end, const TOKEN &ref)
	{
		TRACE_ITEM ti(TRACE_ITEM::Accept, node, begin, end, ref);
		trace.push_back(ti);
	}
public:
	virtual bool enterStatements(boost::int32_t begin)
	{
		enter(NC_Statements, begin);
		return true; // success
	}
	virtual bool rejectStatements(boost::int32_t begin)
	{
		reject(NC_Statements, begin);
		return true; // success
	}
	virtual bool acceptStatements(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_Statements, begin, end);
		return true; // success
	}
	virtual bool enterStatement(boost::int32_t begin)
	{
		enter(NC_Statement, begin);
		return true; // success
	}
	virtual bool rejectStatement(boost::int32_t begin)
	{
		reject(NC_Statement, begin);
		return true; // success
	}
	virtual bool acceptStatement(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_Statement, begin, end);
		return true; // success
	}
	virtual bool enterAssignStatement(boost::int32_t begin)
	{
		enter(NC_AssignStatement, begin);
		return true; // success
	}
	virtual bool rejectAssignStatement(boost::int32_t begin)
	{
		reject(NC_AssignStatement, begin);
		return true; // success
	}
	virtual bool acceptAssignStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
	{
		accept(NC_AssignStatement, begin, end, leftVar);
		return true; // success
	}
	virtual bool enterScanEqStatement(boost::int32_t begin)
	{
		enter(NC_ScanEqStatement, begin);
		return true; // success
	}
	virtual bool rejectScanEqStatement(boost::int32_t begin)
	{
		reject(NC_ScanEqStatement, begin);
		return true; // success
	}
	virtual bool acceptScanEqStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
	{
		accept(NC_ScanEqStatement, begin, end, leftVar);
		return true; // success
	}
	virtual bool enterMatchEqStatement(boost::int32_t begin)
	{
		enter(NC_MatchEqStatement, begin);
		return true; // success
	}
	virtual bool rejectMatchEqStatement(boost::int32_t begin)
	{
		reject(NC_MatchEqStatement, begin);
		return true; // success
	}
	virtual bool acceptMatchEqStatement(boost::int32_t begin, boost::int32_t end, const TOKEN &leftVar)
	{
		accept(NC_MatchEqStatement, begin, end, leftVar);
		return true; // success
	}
	virtual bool enterExpression(boost::int32_t begin)
	{
		enter(NC_Expression, begin);
		return true; // success
	}
	virtual bool rejectExpression(boost::int32_t begin)
	{
		reject(NC_Expression, begin);
		return true; // success
	}
	virtual bool acceptExpression(boost::int32_t begin, boost::int32_t end, const TOKEN &var)
	{
		accept(NC_Expression, begin, end, var);
		return true; // success
	}
	virtual bool enterPattern(boost::int32_t begin)
	{
		enter(NC_Pattern, begin);
		return true; // success
	}
	virtual bool rejectPattern(boost::int32_t begin)
	{
		reject(NC_Pattern, begin);
		return true; // success
	}
	virtual bool acceptPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_Pattern, begin, end);
		return true; // success
	}
	virtual bool enterPackPattern(boost::int32_t begin)
	{
		enter(NC_PackPattern, begin);
		return true; // success
	}
	virtual bool rejectPackPattern(boost::int32_t begin)
	{
		reject(NC_PackPattern, begin);
		return true; // success
	}
	virtual bool acceptPackPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
	{
		accept(NC_PackPattern, begin, end, ope);
		return true; // success
	}
	virtual bool enterMatchPattern(boost::int32_t begin)
	{
		enter(NC_MatchPattern, begin);
		return true; // success
	}
	virtual bool rejectMatchPattern(boost::int32_t begin)
	{
		reject(NC_MatchPattern, begin);
		return true; // success
	}
	virtual bool acceptMatchPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
	{
		accept(NC_MatchPattern, begin, end, ope);
		return true; // success
	}
	virtual bool enterScanPattern(boost::int32_t begin)
	{
		enter(NC_ScanPattern, begin);
		return true; // success
	}
	virtual bool rejectScanPattern(boost::int32_t begin)
	{
		reject(NC_ScanPattern, begin);
		return true; // success
	}
	virtual bool acceptScanPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
	{
		accept(NC_ScanPattern, begin, end, ope);
		return true; // success
	}
	virtual bool enterSequencePattern(boost::int32_t begin)
	{
		enter(NC_SequencePattern, begin);
		return true; // success
	}
	virtual bool rejectSequencePattern(boost::int32_t begin)
	{
		reject(NC_SequencePattern, begin);
		return true; // success
	}
	virtual bool acceptSequencePattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_SequencePattern, begin, end);
		return true; // success
	}
	virtual bool enterOrPattern(boost::int32_t begin)
	{
		enter(NC_OrPattern, begin);
		return true; // success
	}
	virtual bool rejectOrPattern(boost::int32_t begin)
	{
		reject(NC_OrPattern, begin);
		return true; // success
	}
	virtual bool acceptOrPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_OrPattern, begin, end);
		return true; // success
	}
	virtual bool enterRepeatPattern(boost::int32_t begin)
	{
		enter(NC_RepeatPattern, begin);
		return true; // success
	}
	virtual bool rejectRepeatPattern(boost::int32_t begin)
	{
		reject(NC_RepeatPattern, begin);
		return true; // success
	}
	virtual bool acceptRepeatPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &ope)
	{
		accept(NC_RepeatPattern, begin, end, ope);
		return true; // success
	}
	virtual bool acceptRepeatPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_RepeatPattern, begin, end);
		return true; // success
	}
	virtual bool enterAtomPattern(boost::int32_t begin)
	{
		enter(NC_AtomPattern, begin);
		return true; // success
	}
	virtual bool rejectAtomPattern(boost::int32_t begin)
	{
		reject(NC_AtomPattern, begin);
		return true; // success
	}
	virtual bool acceptAtomPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_AtomPattern, begin, end);
		return true; // success
	}
	virtual bool enterParenPattern(boost::int32_t begin)
	{
		enter(NC_ParenPattern, begin);
		return true; // success
	}
	virtual bool rejectParenPattern(boost::int32_t begin)
	{
		reject(NC_ParenPattern, begin);
		return true; // success
	}
	virtual bool acceptParenPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_ParenPattern, begin, end);
		return true; // success
	}
	virtual bool enterXcepPattern(boost::int32_t begin)
	{
		enter(NC_XcepPattern, begin);
		return true; // success
	}
	virtual bool rejectXcepPattern(boost::int32_t begin)
	{
		reject(NC_XcepPattern, begin);
		return true; // success
	}
	virtual bool acceptXcepPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_XcepPattern, begin, end);
		return true; // success
	}
	virtual bool enterPreqPattern(boost::int32_t begin)
	{
		enter(NC_PreqPattern, begin);
		return true; // success
	}
	virtual bool rejectPreqPattern(boost::int32_t begin)
	{
		reject(NC_PreqPattern, begin);
		return true; // success
	}
	virtual bool acceptPreqPattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_PreqPattern, begin, end);
		return true; // success
	}
	virtual bool enterLiteralPattern(boost::int32_t begin)
	{
		enter(NC_LiteralPattern, begin);
		return true; // success
	}
	virtual bool rejectLiteralPattern(boost::int32_t begin)
	{
		reject(NC_LiteralPattern, begin);
		return true; // success
	}
	virtual bool acceptLiteralPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &literal)
	{
		accept(NC_LiteralPattern, begin, end, literal);
		return true; // success
	}
	virtual bool enterGeneratedTokenPattern(boost::int32_t begin)
	{
		enter(NC_GeneratedTokenPattern, begin);
		return true; // success
	}
	virtual bool rejectGeneratedTokenPattern(boost::int32_t begin)
	{
		reject(NC_GeneratedTokenPattern, begin);
		return true; // success
	}
	virtual bool acceptGeneratedTokenPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &generatedToken)
	{
		accept(NC_GeneratedTokenPattern, begin, end, generatedToken);
		return true; // success
	}
	virtual bool enterInsertPattern(boost::int32_t begin)
	{
		enter(NC_InsertPattern, begin);
		return true; // success
	}
	virtual bool rejectInsertPattern(boost::int32_t begin)
	{
		reject(NC_InsertPattern, begin);
		return true; // success
	}
	virtual bool acceptInsertPattern(boost::int32_t begin, boost::int32_t end, const TOKEN &generatedToken)
	{
		accept(NC_InsertPattern, begin, end, generatedToken);
		return true; // success
	}
	virtual bool enterRecursePattern(boost::int32_t begin)
	{
		enter(NC_RecursePattern, begin);
		return true; // success
	}
	virtual bool rejectRecursePattern(boost::int32_t begin)
	{
		reject(NC_RecursePattern, begin);
		return true; // success
	}
	virtual bool acceptRecursePattern(boost::int32_t begin, boost::int32_t end)
	{
		accept(NC_RecursePattern, begin, end);
		return true; // success
	}
public:
	static std:: vector<TRACE_ITEM> reduce(const std:: vector<TRACE_ITEM> &trace0)
	{
		std:: vector<TRACE_ITEM> trace;
		trace.reserve(trace0.size());

		for (size_t i = 0; i < trace0.size(); ++i) {
			const TRACE_ITEM &item = trace0[i];
			switch (item.classification) {
			case TRACE_ITEM::Enter:
				{
					trace.push_back(item);
				}
				break;
			case TRACE_ITEM::Reject:
				{
					while (trace.size() > 0) {
						TRACE_ITEM &ti = trace.back();
						if (ti.begin == item.begin) {
							break; // while
						}
						trace.pop_back();
					}
					
					if (trace.size() > 0) {
						TRACE_ITEM &ti = trace[trace.size() - 1];
						assert(ti.classification == TRACE_ITEM::Enter);
						assert(ti.node == item.node);
						assert(ti.begin == item.begin);
						trace.pop_back();
					}
					else {
						assert(false);
					}
				}
				break;
			case TRACE_ITEM::Accept:
				{
					trace.push_back(item);
				}
				break;
			}
		}
		return trace;
	}
};


class TraceDirector {
private:
	std:: vector<TRACE_ITEM> trace;
	std:: vector<boost::int32_t> pairInfo;
public:
	void setTrace(const std:: vector<TRACE_ITEM> &trace_)
	{
		trace = trace_;
		pairInfo = findPairs(trace);
	}
	const std:: vector<TRACE_ITEM> &refTrace() const
	{
		return trace;
	}
	TRACE_ITEM getAt(boost::int32_t idx) const
	{
		assert(0 <= idx && idx <= trace.size());
		return trace[idx];
	}
	boost::int32_t findPair(boost::int32_t idx) const
	{
		assert(0 <= idx && idx <= trace.size());
		return pairInfo[idx];
	}
	boost::int32_t findNext(boost::int32_t idx) const // return -1 if no next exists
	{
		assert(0 <= idx && idx <= trace.size());
		const TRACE_ITEM &item = trace[idx];
		boost::int32_t closeIdx;
		if (item.classification == TRACE_ITEM::Enter) {
			closeIdx = pairInfo[idx];
			assert(trace[closeIdx].classification == TRACE_ITEM::Reject || trace[closeIdx].classification == TRACE_ITEM::Accept);
		}
		else {
			closeIdx = idx;
		}
		if (! ((closeIdx + 1) < trace.size() && trace[closeIdx + 1].classification == TRACE_ITEM::Enter)) {
			return -1; // no next exists
		}
		return closeIdx + 1;
	}
	boost::int32_t findPrev(boost::int32_t idx) const // return -1 if no prev exists
	{
		assert(0 <= idx && idx <= trace.size());
		const TRACE_ITEM &item = trace[idx];
		boost::int32_t openIdx;
		if (item.classification != TRACE_ITEM::Enter) {
			openIdx = pairInfo[idx];
			assert(trace[openIdx].classification == TRACE_ITEM::Enter);
		}
		else {
			openIdx = idx;
		}
		if (! ((openIdx - 1) >= 0 && (trace[openIdx - 1].classification == TRACE_ITEM::Reject || trace[openIdx - 1].classification == TRACE_ITEM::Accept))) {
			return -1; // no prev exists
		}
		return pairInfo[openIdx - 1];
	}
	boost::int32_t findParent(boost::int32_t idx) const // return -1 if no parent exists
	{
		assert(0 <= idx && idx <= trace.size());
		const TRACE_ITEM &item = trace[idx];
		boost::int32_t openIdx;
		if (item.classification != TRACE_ITEM::Enter) {
			openIdx = pairInfo[idx];
			assert(trace[openIdx].classification == TRACE_ITEM::Enter);
		}
		else {
			openIdx = idx;
		}
		boost::int32_t cur = openIdx;
		boost::int32_t p;
		while ((p = findPrev(cur)) != -1) {
			cur = p;
		}
		if (cur == 0) {
			return -1; // no parent exists
		}
		assert(trace[cur - 1].classification == TRACE_ITEM::Enter);
		return cur - 1;
	}
	std:: vector<boost::int32_t> findChildren(boost::int32_t idx) const
	{
		assert(0 <= idx && idx <= trace.size());
		const TRACE_ITEM &item = trace[idx];
		boost::int32_t openIdx;
		if (item.classification != TRACE_ITEM::Enter) {
			openIdx = pairInfo[idx];
			assert(trace[openIdx].classification == TRACE_ITEM::Enter);
		}
		else {
			openIdx = idx;
		}
		std:: vector<boost::int32_t> r;
		if (trace[openIdx + 1].classification != TRACE_ITEM::Enter) {
			return r;
		}
		boost::int32_t cidx = openIdx + 1;
		r.push_back(cidx);
		boost::int32_t p;
		while ((p = findNext(cidx)) != -1) {
			cidx = p;
			r.push_back(cidx);
		}
		return r;
	}
private:
	static std:: vector<boost::int32_t> findPairs(const std:: vector<TRACE_ITEM> &trace)
	{
		std:: vector<boost::int32_t> pairs;
		pairs.resize(trace.size());

		std:: vector<boost::int32_t> stack;
		for (boost::int32_t i = 0; i < trace.size(); ++i) {
			const TRACE_ITEM &item = trace[i];
			switch (item.classification) {
			case TRACE_ITEM::Enter:
				{
					stack.push_back(i);
				}
				break;
			case TRACE_ITEM::Reject:
			case TRACE_ITEM::Accept:
				{
					assert(stack.size() > 0);
					boost::int32_t pairIdx = stack.back();
					stack.pop_back();
					const TRACE_ITEM &pairItem = trace[pairIdx];
					assert(pairItem.begin == item.begin);
					assert(pairItem.node == item.node);
					pairs[i] = pairIdx;
					pairs[pairIdx] = i;
				}
				break;
			default:
				assert(false);
				break;
			}
		}
		assert(stack.size() == 0);
		return pairs;
	}
};

#endif // TORQPARSER_H
