#if ! defined Token_H
#define Token_H

#include <vector>
#include <map>
#include <set>
#include "../common/hash_map_includer.h"

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>
#include <boost/pool/object_pool.hpp>

#include "torqcommon.h"

namespace text {

struct RawCharToken;
struct GeneratedToken;

struct Token {
public:
	virtual ~Token()
	{
	}
	virtual void destroy()
	{
		assert(false); // this code must not be executed
	}
public:
	virtual Token *dup() const = 0;
	virtual bool operator==(const Token &right) const = 0;

	virtual boost::optional<boost::int32_t> getGeneratedCode() const = 0;
	virtual boost::optional<MYWCHAR_T> getRawCharCode() const = 0;

	bool isRawChar() const { return !! getRawCharCode(); }
	bool isGenerated() const { return !! getGeneratedCode(); }

	GeneratedToken *castToGenerated() { return isGenerated() ? (GeneratedToken *)this : NULL; }
	const GeneratedToken *castToGenerated() const { return isGenerated() ? (const GeneratedToken *)this : NULL; }
};

struct RawCharToken : public Token {
public:
	static RawCharToken *create(MYWCHAR_T code_, boost::int32_t pos_);

public:
	virtual boost::optional<boost::int32_t> getGeneratedCode() const
	{
		return boost::optional<boost::int32_t>();
	}
	virtual void destroy()
	{
		assert(false); // this code must not be executed
	}
};

/* 
This class is very similar to boost::ptr_vector<boost::nullable<Token> >.
The reason why this class exists is that boost::ptr_vector<boost::nullable<Token> >::replace() does not accept NULL value,
possbly bug.
*/
class TokenSequence {
private:
	std::vector<Token *> body;
public:
	~TokenSequence()
	{
		for (size_t i = 0; i < body.size(); ++i) {
			Token *p = body[i];
			if (p != NULL) {
				p->destroy();
			}
		}
	}
	TokenSequence(const TokenSequence &right)
	{
		body.resize(right.body.size(), NULL);
		for (size_t i = 0; i < right.body.size(); ++i) {
			const Token *p = right.body[i];
			if (p != NULL) {
				body[i] = p->dup();
			}
		}
	}
	TokenSequence()
	{
	}
	void swap(TokenSequence &right)
	{
		body.swap(right.body);
	}
public:
	TokenSequence &operator=(const TokenSequence &right)
	{
		if (&right == this) {
			return *this;
		}

		for (size_t i = 0; i < body.size(); ++i) {
			Token *p = body[i];
			if (p != NULL) {
				p->destroy();
			}
		}
		body.resize(0);

		body.resize(right.body.size(), NULL);
		for (size_t i = 0; i < right.body.size(); ++i) {
			const Token *p = right.body[i];
			if (p != NULL) {
				body[i] = p->dup();
			}
		}

		return *this;
	}
public:
	void resize(size_t newSize)
	{
		size_t curSize = size();
		if (newSize > curSize) {
			body.resize(newSize, NULL);
		}
		else {
			for (size_t i = newSize; i < curSize; ++i) {
				Token *p = body[i];
				if (p != NULL) {
					p->destroy();
				}
			}
			body.resize(newSize);
		}
	}
	void reserve(size_t rSize)
	{
		body.reserve(rSize);
	}
	void clear()
	{
		resize(0);
	}
public:
	size_t size() const
	{
		return body.size();
	}
	Token *refAt(size_t index) const
	{
		assert(index < body.size());
		return body[index];
	}
	Token *refBack() const
	{
		assert(body.size() >= 1);
		return body.back();
	}
	void attachBack(Token *pToken)
	{
		body.push_back(pToken);
	}
	Token *replaceAt(size_t index, Token *pToken)
	{
		assert(index < body.size());
		Token *p = body[index];
		body[index] = pToken;
		return p;
	}
public:
	bool operator==(const TokenSequence &right) const
	{
		if (body.size() != right.body.size()) {
			return false;
		}
		
		for (size_t i = 0; i < body.size(); ++i) {
			const Token *pL = body[i];
			const Token *pR = right.body[i];
			if (pL == pR) {
				NULL;
			}
			else {
				if (pL == NULL || pR == NULL) {
					return false;
				}
				else {
					if (pL->operator==(*pR)) {
						NULL;
					}
					else {
						return false;
					}
				}
			}
		}
		return true;
	}
};

struct GeneratedToken : public Token {
public:
	static const boost::int32_t/* code */ cNULL;

#if defined USE_BOOST_POOL
protected:
	static boost::object_pool<GeneratedToken> ThePool;
#endif

public:
	boost::int32_t code;
	TokenSequence value;
public:
	GeneratedToken()
		: code(0), value()
	{
	}
protected:
	GeneratedToken(const GeneratedToken &right)
		: code(right.code), value(right.value)
	{
	}
	GeneratedToken(boost::int32_t code_, const TokenSequence &value_)
		: code(code_), value(value_)
	{
	}
public:
	static GeneratedToken *create()
	{
#if defined USE_BOOST_POOL
		return GeneratedToken::ThePool.construct();
#else
		return new GeneratedToken();
#endif
	}
	static GeneratedToken *create(boost::int32_t code_, const TokenSequence &value_)
	{
#if defined USE_BOOST_POOL
		GeneratedToken *p = GeneratedToken::ThePool.construct();
		p->code = code_;
		p->value = value_;
		return p;
#else
		return new GeneratedToken(code_, value_);
#endif
	}
	virtual void destroy()
	{
#if defined USE_BOOST_POOL
		GeneratedToken::ThePool.destroy(this);
#else
		delete this;
#endif
	}
	void swap(GeneratedToken &right)
	{
		std:: swap(this->code, right.code);
		this->value.swap(right.value);
	}
public:
	virtual Token *dup() const
	{
#if defined USE_BOOST_POOL
		GeneratedToken *p = GeneratedToken::ThePool.construct();
		p->code = code;
		p->value = value;
		return p;
#else
		return new GeneratedToken(*this);
#endif
	}
	virtual bool operator==(const Token &right) const
	{
		const GeneratedToken *pRight = right.castToGenerated();

		if (pRight == this) {
			return true;
		}

		if (this->code == pRight->code) {
			if (this->value == pRight->value) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	virtual boost::optional<MYWCHAR_T> getRawCharCode() const 
	{
		return boost::optional<MYWCHAR_T>();
	}
	virtual boost::optional<boost::int32_t> getGeneratedCode() const
	{
		return boost::optional<boost::int32_t>(code);
	}
public:
	static const std:: vector<std:: pair<std:: vector<MYWCHAR_T>/* name */, GeneratedToken *> > SpecialTokens;
};

class Helper
{
public:
	static void buildTokenSequence(TokenSequence *pSeq, std:: vector<MYWCHAR_T> &originalSeq, bool convertNewLineChar, bool convertEOF)
	{
		TokenSequence seq;
		seq.reserve(originalSeq.size());
		size_t i = 0; 
		while (i < originalSeq.size()) {
			MYWCHAR_T ch = originalSeq[i];
			switch (ch) {
			case '\r':
			case '\n':
				{
					if (! convertNewLineChar) {
						seq.attachBack(RawCharToken::create(ch, i));
						++i;
					}
					else {
						TokenSequence eolSeq;
						if (ch == '\r') {
							if (i + 1 < originalSeq.size() && (ch = originalSeq[i + 1]) == '\n') {
								eolSeq.attachBack(RawCharToken::create('\r', i));
								eolSeq.attachBack(RawCharToken::create('\n', i + 1));
								i += 2;
							}
							else {
								eolSeq.attachBack(RawCharToken::create('\r', i));
								++i;
							}
						}
						else if (ch == '\n') {
							eolSeq.attachBack(RawCharToken::create('\n', i));
							++i;
						}
						text::GeneratedToken *pEOL = (text::GeneratedToken*)text::GeneratedToken::SpecialTokens[3/* eol */].second->dup();
						pEOL->value.swap(eolSeq);
						seq.attachBack(pEOL);
					}
				}
				break;
			case '\x1a': // eof
				{
					if (! convertEOF) {
						seq.attachBack(RawCharToken::create(ch, i));
						++i;
					}
					else {
						TokenSequence eofSeq;
						eofSeq.attachBack(RawCharToken::create('\x1a', i));
						++i;
						text::GeneratedToken *pEOF = (text::GeneratedToken*)text::GeneratedToken::SpecialTokens[2/* eof */].second->dup();
						pEOF->value.swap(eofSeq);
						seq.attachBack(pEOF);
					}
				}
				break;
			default:
				{
					seq.attachBack(RawCharToken::create(ch, i));
					++i;
				}
				break;
			}
		}
		if (convertEOF && originalSeq.size() >= 1 && originalSeq.back() != '\x1a'/* eof */) {
			seq.attachBack(text::GeneratedToken::SpecialTokens[2/* eof */].second->dup());
		}
		
		(*pSeq).swap(seq);
	}
public:
	enum PrintOptions {
		RecurseNull = (1 << 1),
		NewLineThru = (1 << 2),
		SkipNull = (1 << 3),
	};
public:
	enum NF_NODETYPE {
		NF_NONE, NF_TERMINATED, NF_EXPANDED
	};
	struct NodeFormat {
	public:
		NF_NODETYPE nodeType;
		std:: vector<MYWCHAR_T> opening;
		std:: vector<MYWCHAR_T> closing;
	public:
		NodeFormat(NF_NODETYPE nodeType_, const std:: vector<MYWCHAR_T> &opening_, const std:: vector<MYWCHAR_T> &closing_)
			: nodeType(nodeType_), opening(opening_), closing(closing_)
		{
		}
		NodeFormat()
			: nodeType(NF_NONE), opening(), closing()
		{
		}
		NodeFormat(const NodeFormat &right)
			: nodeType(right.nodeType), opening(right.opening), closing(right.closing)
		{
		}
	public:
		void swap(NodeFormat &right)
		{
			std:: swap(nodeType, right.nodeType);
			opening.swap(right.opening);
			closing.swap(right.closing);
		}
	};
private:
	struct NodeFormatI {
	public:
		NF_NODETYPE nodeType;
		std:: string opening;
		std:: string closing;
	public:
		NodeFormatI(NF_NODETYPE nodeType_, const std:: string &opening_, const std:: string &closing_)
			: nodeType(nodeType_), opening(opening_), closing(closing_)
		{
		}
		NodeFormatI()
			: nodeType(NF_NONE), opening(), closing()
		{
		}
		NodeFormatI(const NodeFormatI &right)
			: nodeType(right.nodeType), opening(right.opening), closing(right.closing)
		{
		}
	public:
		void swap(NodeFormatI &right)
		{
			std:: swap(nodeType, right.nodeType);
			opening.swap(right.opening);
			closing.swap(right.closing);
		}
	};
	struct PrintData {
		std:: ostream *pOutput;
		std:: string separator;
        unsigned long options;
		const std:: vector<std:: pair<bool/* is valid */, NodeFormatI> > *pNodeFormats;
		const common::Encoder *pRawCharEncoder;
		text::Helper::NodeFormatI rawOpenClose;
		//std:: string encodedNewLine;
		std:: string bra;
		std:: string ket;
	};
public:
	static void print(std:: ostream *pOutput, const TokenSequence &text, const std:: vector<MYWCHAR_T> &separator0,	
			unsigned long options, const common::Encoder *pRawCharEncoder0)
	{
		PrintData data;
		data.pOutput = pOutput;
		data.separator = common::EscapeSequenceHelper::encode(separator0, false);
		data.options = options;
		data.pNodeFormats = NULL;
		data.pRawCharEncoder = pRawCharEncoder0 != NULL ? pRawCharEncoder0 : new common::HTMLEncoder();
		data.rawOpenClose = text::Helper::NodeFormatI(NF_EXPANDED, "\"", "\"");
		//if ((data.options & NewLineThru) != 0) {
		//	data.encodedNewLine = "\n";
		//}
		//else {
		//	switch (data.rawOpenClose.nodeType) {
		//	case text::Helper::NF_EXPANDED:
		//		data.encodedNewLine = data.rawOpenClose.opening + data.pRawCharEncoder->encode('\n') + data.rawOpenClose.closing + data.separator;
		//		break;
		//	case text::Helper::NF_TERMINATED:
		//		assert(! data.rawOpenClose.opening.empty());
		//		data.encodedNewLine = data.rawOpenClose.opening + data.separator;
		//		break;
		//	case text::Helper::NF_NONE:
		//		data.encodedNewLine = "";
		//		break;
		//	default:
		//		assert(false);
		//		break;
		//	}
		//}
		data.bra = "[";
		data.ket = "]";
		std:: pair<size_t/* row */, size_t/* col */> rowCol(1, 1);
		size_t index = 0;
		print_i(&rowCol, &index, text, data);
		if (pRawCharEncoder0 == NULL) {
			delete data.pRawCharEncoder;
		}
	}
	static void print(std:: ostream *pOutput, const TokenSequence &text, 
			const NodeFormat &rawTextFormat, const std:: vector<MYWCHAR_T> &separator, unsigned long options, 
			const HASH_MAP<boost::int32_t/* code */, NodeFormat> &nodeFormats, 
			const common::Encoder *pRawCharEncoder0, const common::Encoder *pGeneratedEncoder0)
	{
		PrintData data;
		data.pOutput = pOutput;
		data.options = options;
		data.pRawCharEncoder = pRawCharEncoder0 != NULL ? pRawCharEncoder0 : new common::HTMLEncoder();
		const common::Encoder *pGeneratedEncoder = pGeneratedEncoder0 != NULL ? pGeneratedEncoder0 : new common::UTF8NEncoder();
		
		boost::int32_t maxCode = 0;
		{
			for (HASH_MAP<boost::int32_t/* code */, NodeFormat>::const_iterator i = nodeFormats.begin(); i != nodeFormats.end(); ++i) {
				if (i->first > maxCode) {
					maxCode = i->first;
				}
			}
		}

		std:: vector<std:: pair<bool/* is valid */, NodeFormatI> > encodedNodeFormats;
		encodedNodeFormats.resize(maxCode + 1);
		for (HASH_MAP<boost::int32_t/* code */, NodeFormat>::const_iterator i = nodeFormats.begin(); i != nodeFormats.end(); ++i) {
			if (i->first >= 0) {
				const NodeFormat &n = i->second;
				std:: string op = pGeneratedEncoder->encode(n.opening);
				std:: string cl = pGeneratedEncoder->encode(n.closing);
				NodeFormatI encoded(n.nodeType, op, cl);
				encodedNodeFormats[i->first].first = true;
				encodedNodeFormats[i->first].second.swap(encoded);
			}
		}

		data.pNodeFormats = &encodedNodeFormats;
		
		data.separator = pGeneratedEncoder->encode(separator);
		data.rawOpenClose.nodeType = rawTextFormat.nodeType;
		data.rawOpenClose.opening = pGeneratedEncoder->encode(rawTextFormat.opening);
		data.rawOpenClose.closing = pGeneratedEncoder->encode(rawTextFormat.closing);

		//if ((data.options & NewLineThru) != 0) {
		//	data.encodedNewLine = "\n";
		//}
		//else {
		//	switch (data.rawOpenClose.nodeType) {
		//	case text::Helper::NF_EXPANDED:
		//		data.encodedNewLine = data.rawOpenClose.opening + data.pRawCharEncoder->encode('\n') + data.rawOpenClose.closing + data.separator;
		//		break;
		//	case text::Helper::NF_TERMINATED:
		//		assert(! data.rawOpenClose.opening.empty());
		//		data.encodedNewLine = data.rawOpenClose.opening + data.separator;
		//		break;
		//	case text::Helper::NF_NONE:
		//		data.encodedNewLine = "";
		//		break;
		//	default:
		//		assert(false);
		//		break;
		//	}
		//}
		
		data.bra = "[";
		data.ket = "]";
		
		std:: pair<size_t/* row */, size_t/* col */> rowCol(1, 1);
		size_t index = 0;
		print_i(&rowCol, &index, text, data);
		
		if (pRawCharEncoder0 == NULL) {
			delete data.pRawCharEncoder;
		}
		if (pGeneratedEncoder0 == NULL) {
			delete pGeneratedEncoder;
		}
	}
private:
	static void write(std:: ostream *pOutput, const std:: string &str)
	{
		(*pOutput).write(str.data(), str.length());
	}
	static std:: string expandSpecials(const std:: string &format, 
			const std:: pair<size_t/* row */, size_t/* col */> &rowCol, size_t index)
	{
		std:: string s;

		size_t i = 0;
		while (i < format.size()) {
			size_t j = format.find('%', i);
			if (j == std:: string::npos) {
				if (i == 0) {
					return format;
				}
				else {
					s += format.substr(i);
					i = format.size();
				}
			}
			else {
				s += format.substr(i, j - i);
				if (! (j + 1 < format.size())) {
					s += '%';
					i = j + 1;
				}
				else {
					switch (format[j + 1]) {
					case '%':
						s += '%';
						break;
					case 'r':
						s += (boost::format("%d") % rowCol.first).str();
						break;
					case 'c':
						s += (boost::format("%d") % rowCol.second).str();
						break;
					case 'N':
						s += '\n';
						break;
					case 'i':
						s += (boost::format("%d") % index).str();
						break;
					case 'S':
						s += ' ';
						break;
					case 'T':
						s += '\t';
						break;
					default:
						assert(false);
						break;
					}
					i = j + 2;
				}
			}
		}

		return s;
	}
	static void print_i(std:: pair<size_t/* row */, size_t/* col */> *pRowCol, size_t *pIndex, 
			const TokenSequence &text, const PrintData &data)
	{
		std:: pair<size_t/* row */, size_t/* col */> &rowCol = *pRowCol;
		size_t &index = *pIndex;

		std:: ostream &output = *data.pOutput;

		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *pToken = text.refAt(i);
			if (pToken == NULL) {
				write(&output, "NULL");
				write(&output, data.separator);
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = pToken->getRawCharCode();
			if (r) {
				write(&output, expandSpecials(data.rawOpenClose.opening, rowCol, index));
				MYWCHAR_T ch = *r;
				write(&output, data.pRawCharEncoder->encode(ch));
				++i;
				++index;
				++rowCol.second;
				if (ch == '\r') {
					++rowCol.first;
					rowCol.second = 1;
					if (i < text.size() && (pToken = text.refAt(i)) != NULL && (r = pToken->getRawCharCode()) && (ch = *r) == '\n') {
						write(&output, data.pRawCharEncoder->encode(ch));
						++index;
					}
					write(&output, expandSpecials(data.rawOpenClose.closing, rowCol, index));
					write(&output, data.separator);
					continue;
				}
				else if (ch == '\n') {
					++rowCol.first;
					rowCol.second = 1;
					write(&output, expandSpecials(data.rawOpenClose.closing, rowCol, index));
					write(&output, data.separator);
					continue;
				}

				const text::Token *pToken;
				while (i < text.size() && (pToken = text.refAt(i)) != NULL && (r = pToken->getRawCharCode()) && (ch = *r) != '\n' && ch != '\r') {
					write(&output, data.pRawCharEncoder->encode(ch));
					++i;
					++index;
					++rowCol.second;
				}
				write(&output, expandSpecials(data.rawOpenClose.closing, rowCol, index));
				write(&output, data.separator);
				continue;
			}
			const text::GeneratedToken *g = pToken->castToGenerated();
			if (g != NULL) {
				if (g->code == 0 && (data.options & SkipNull) != 0) {
					NULL;
				}
				else if (g->code == 3/* EOL */ && (data.options & NewLineThru) != 0) {
					output << std:: endl;
					const TokenSequence &value = g->value;
					print_i_silent(&rowCol, &index, value);
				}
				else {
					if (data.pNodeFormats != NULL && g->code < (*data.pNodeFormats).size() && (*data.pNodeFormats)[g->code].first) {
						const TokenSequence &value = g->value;
						const NodeFormatI &openClose = (*data.pNodeFormats)[g->code].second;
						switch (openClose.nodeType) {
						case text::Helper::NF_EXPANDED:
							{
								if (openClose.opening.size() > 0) {
									write(&output, expandSpecials(openClose.opening, rowCol, index));
									write(&output, data.separator);
								}
								print_i(&rowCol, &index, value, data);
								if (openClose.closing.size() > 0) {
									write(&output, expandSpecials(openClose.closing, rowCol, index));
									write(&output, data.separator);
								}
							}
							break;
						case text::Helper::NF_TERMINATED:
							{
								static const std:: string PERCENT_S = "%s";
								assert(openClose.opening.size() > 0);
								size_t p = openClose.opening.find(PERCENT_S);
								if (p != std:: string::npos) {
									write(&output, expandSpecials(openClose.opening.substr(0, p), rowCol, index));
									print_i_leaftext(&rowCol, &index, value, data);
									write(&output, expandSpecials(openClose.opening.substr(p + PERCENT_S.length()), rowCol, index));
									write(&output, data.separator);
								}
								else {
									write(&output, expandSpecials(openClose.opening, rowCol, index));
									print_i_silent(&rowCol, &index, value);
									write(&output, data.separator);
								}
							}
							break;
						case text::Helper::NF_NONE:
							{
								print_i_silent(&rowCol, &index, value);
							}
							break;
						default:
							assert(false);
							break;
						}
					}
					else {
						output << g->code;
						if (g->code != 0 || g->code == 0 && (data.options & RecurseNull) != 0) {
							write(&output, data.bra);
							write(&output, data.separator);
							const TokenSequence &value = g->value;
							print_i(&rowCol, &index, value, data);
							write(&output, data.ket);
							write(&output, data.separator);
						}
					}
				}
				++i;
			}
		}
	}
private:
	struct PrintDataXml {
		std:: ostream *pOutput;
        unsigned long options;
		std:: string rawTokenTagName;
		const std:: vector<std:: string> *pNodeNames;
	};
	static void printXml_i(const TokenSequence &text, const PrintDataXml &data)
	{
		std:: ostream &output = *data.pOutput;
		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *p = text.refAt(i);
			if (p == NULL) {
				output << "<NULL/>" << std:: endl;
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = p->getRawCharCode();
			if (r) {
				MYWCHAR_T ch = *r;
				if (ch == '\r') {
					output << "<" << data.rawTokenTagName << ">" 
							<< common::EscapeSequenceHelper::encode(ch, true) 
							<< "</" << data.rawTokenTagName << ">" << std:: endl;
					++i;
				}
				else if (ch == '\n') {
					output << "<" << data.rawTokenTagName << ">" 
							<< common::EscapeSequenceHelper::encode(ch, true) 
							<< "</" << data.rawTokenTagName << ">" << std:: endl;
					++i;
				}
				else {
					output << "<" << data.rawTokenTagName << ">"
							<< common::EscapeSequenceHelper::encode(ch, true);
					++i;
					const text::Token *pToken;
					while (i < text.size() && (pToken = text.refAt(i)) != NULL && (r = pToken->getRawCharCode()) && (ch = *r) != '\n') {
						output << common::EscapeSequenceHelper::encode(ch, true);
						++i;
					}
					output << "</" << data.rawTokenTagName << ">" << std:: endl;
				}
				continue;
			}
			const text::GeneratedToken *g = p->castToGenerated();
			if (g != NULL) {
				boost::int32_t code = g->code;
				if (code == 0 && (data.options & SkipNull) != 0) {
					NULL;
				}
				else {
					if (0 <= code && code < data.pNodeNames->size()) {
						output << "<" << (*data.pNodeNames)[code] << ">" << std:: endl;
					}
					else {
						output << "<" << code << ">" << std:: endl;
					}
					if (code != 0 || code == 0 && (data.options & RecurseNull) != 0) {
						const TokenSequence &value = g->value;
						printXml_i(value, data);
					}
					if (0 <= code && code < data.pNodeNames->size()) {
						output << "</" << (*data.pNodeNames)[code] << ">" << std:: endl;
					}
					else {
						output << "</" << code << ">" << std:: endl;
					}
				}
				++i;
			}
		}
	}
public:
	static void printXml(std:: ostream *pOutput, const TokenSequence &text, const std:: vector<MYWCHAR_T> &rawTokenTagName, 
			unsigned long options, const std:: vector<std:: vector<MYWCHAR_T> > &nodeNames)
	{
		PrintDataXml data;
		data.pOutput = pOutput;
		data.options = options;
		data.rawTokenTagName = common::EscapeSequenceHelper::encode(rawTokenTagName, false);
		
		std:: vector<std:: string> encodedNodeNames;
		encodedNodeNames.reserve(nodeNames.size());
		for (size_t i = 0; i < nodeNames.size(); ++i) {
			encodedNodeNames.push_back(common::EscapeSequenceHelper::encode(nodeNames[i], false));
		}

		data.pNodeNames = &encodedNodeNames;
		(*pOutput) << "<xml>" << std:: endl;
		printXml_i(text, data);
		(*pOutput) << "</xml>" << std:: endl;
	}
public:
	static void printCng(std:: ostream *pOutput, const TokenSequence &text, const HASH_MAP<boost::int32_t/* code */, NodeFormat> &nodeFormats)
	{
		boost::int32_t maxCode = 0;
		{
			for (HASH_MAP<boost::int32_t/* code */, NodeFormat>::const_iterator i = nodeFormats.begin(); i != nodeFormats.end(); ++i) {
				if (i->first > maxCode) {
					maxCode = i->first;
				}
			}
		}
		std:: vector<std:: pair<bool/* is valid */, NodeFormatI> > encodedNodeFormats;
		encodedNodeFormats.resize(maxCode + 1);
		for (HASH_MAP<boost::int32_t/* code */, NodeFormat>::const_iterator i = nodeFormats.begin(); i != nodeFormats.end(); ++i) {
			if (i->first >= 0) {
				const NodeFormat &n = i->second;
				NodeFormatI encoded(
						n.nodeType,
						common::EscapeSequenceHelper::encode(n.opening, false), 
						common::EscapeSequenceHelper::encode(n.closing, false));
				encodedNodeFormats[i->first].first = true;
				encodedNodeFormats[i->first].second.swap(encoded);
			}
		}

		std:: pair<size_t/* row */, size_t/* col */> rowCol(1, 1);
		size_t index = 0;
		printCng_i(pOutput, &rowCol, &index, text, encodedNodeFormats);
	}
private:
	static void printCng_i(std:: ostream *pOutput, std:: pair<size_t/* row */, size_t/* col */> *pRowCol, size_t *pIndex,
			const TokenSequence &text, const std:: vector<std:: pair<bool/* is valid */, NodeFormatI> > &encodedNodeFormats)
	{
		std:: ostream &output = *pOutput;
		std:: pair<size_t/* row */, size_t/* col */> &rowCol = *pRowCol;
		size_t &index = *pIndex;
		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *p = text.refAt(i);
			if (p == NULL) {
				output << "NULL" << std:: endl;
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = p->getRawCharCode();
			if (r) {
				MYWCHAR_T ch = *r;
				if (ch == '\r') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
					if (i < text.size()) {
						r = text.refAt(i)->getRawCharCode();
						if (r && *r == '\n') {
							++index;
							++i;
						}
					}
				}
				else if (ch == '\n') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
				}
				else {
					size_t iFrom = i;
					std:: pair<size_t/* row */, size_t/* col */> rcFrom = rowCol;
					size_t indexFrom = index;
					//MYWCHAR_T rCode = r->getCode();

					++rowCol.second;
					++index;
					++i;
					const text::Token *pToken;
					MYWCHAR_T ch;
					while (i < text.size() && (pToken = text.refAt(i)) != NULL && (r = pToken->getRawCharCode()) && (ch = *r) != '\n' && ch != '\r') {
						++rowCol.second;
						++index;
						++i;
					}
					size_t iTo = i;
					std:: pair<size_t/* row */, size_t/* col */> rcTo = rowCol;
					size_t indexTo = index;

					output << (boost::format("%x.%x.%x\t%x.%x.%x\t") % rcFrom.first % rcFrom.second % indexFrom % rcTo.first % rcTo.second % indexTo)
							<< "\"";
					for (size_t j = iFrom; j < iTo; ++j) {
						output << common::EscapeSequenceHelper::encode(*text.refAt(j)->getRawCharCode(), false);
					}
					output << "\"" << std:: endl;
				}
				continue;
			}
			const text::GeneratedToken *g = p->castToGenerated();
			if (g != NULL) {
				boost::int32_t gCode = g->code;
				if (gCode == text::GeneratedToken::cNULL) {
					const TokenSequence &value = g->value;
					print_i_silent(&rowCol, &index, value);
				}
				else {
					if (0 <= gCode && gCode < encodedNodeFormats.size() && encodedNodeFormats[gCode].first) {
						const NodeFormatI &openClose = encodedNodeFormats[gCode].second;
						switch (openClose.nodeType) {
						case text::Helper::NF_EXPANDED:
							{
								if (! openClose.opening.empty()) {
									output << (boost::format("%x.%x.%x\t+0\t") % rowCol.first % rowCol.second % index) 
											<< openClose.opening << std:: endl;
								}
								const TokenSequence &value = g->value;
								printCng_i(pOutput, &rowCol, &index, value, encodedNodeFormats);
								if (! openClose.closing.empty()) {
									output << (boost::format("%x.%x.%x\t+0\t") % rowCol.first % rowCol.second % index) 
											<< openClose.closing << std:: endl;
								}
							}
							break;
						case text::Helper::NF_TERMINATED:
							{
								if (! openClose.opening.empty()) {
									output << (boost::format("%x.%x.%x\t") % rowCol.first % rowCol.second % index);
								}
								std:: pair<size_t/* row */, size_t/* col */> lastRowCol = rowCol;
								size_t lastIndex = index;
								const TokenSequence &value = g->value;
								std:: pair<size_t/* row */, size_t/* col */> rowColLTE = rowCol;
								size_t indexLTE = index;
								print_i_silent(&rowCol, &rowColLTE, &index, &indexLTE, value);
								if (! openClose.opening.empty()) {
									int indexDiff = indexLTE - lastIndex;
									if (rowColLTE.first == lastRowCol.first && rowColLTE.second - lastRowCol.second == indexDiff) {
										output << (boost::format("+%x\t") % indexDiff);
									}
									else {
										output << (boost::format("%x.%x.%x\t") % rowColLTE.first % rowColLTE.second % indexLTE);
									}
									static const std:: string PERCENT_S = "%s";
									size_t p = openClose.opening.find(PERCENT_S);
									if (p != std:: string::npos) {
										output << openClose.opening.substr(0, p);
										printCng_i_leaftext(&output, value);
										output << openClose.opening.substr(p + PERCENT_S.length());
									}
									else {
										output << openClose.opening;
									}
									output << std:: endl;
								}
							}
							break;
						case text::Helper::NF_NONE:
							{
								const TokenSequence &value = g->value;
								print_i_silent(&rowCol, &index, value);
							}
							break;
						default:
							assert(false);
							break;
						}
					}
					else {
						output << (boost::format("%x.%x.%x\t+0\t") % rowCol.first % rowCol.second % index)
								<< g->code << std:: endl;
						const TokenSequence &value = g->value;
						print_i_silent(&rowCol, &index, value);
					}
				}
				++i;
				continue;
			}
		}
	}
	static void print_i_silent(
			std:: pair<size_t/* row */, size_t/* col */> *pRowCol, 
			size_t *pIndex,
			const TokenSequence &text)
	{
		print_i_silent(pRowCol, NULL, pIndex, NULL, text);
	}
	static void print_i_silent(
			std:: pair<size_t/* row */, size_t/* col */> *pRowCol, 
			std:: pair<size_t/* row */, size_t/* col */> *pRowColLastNonNull, 
			size_t *pIndex, size_t *pIndexLastNonNull,
			const TokenSequence &text)
	{
		assert(pRowCol != NULL);
		assert(pIndex != NULL);
		std:: pair<size_t/* row */, size_t/* col */> &rowCol = *pRowCol;
		size_t &index = *pIndex;
		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *p = text.refAt(i);
			if (p == NULL) {
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = p->getRawCharCode();
			if (r) {
				MYWCHAR_T ch = *r;
				if (ch == '\r') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
					if (i < text.size()) {
						r = text.refAt(i)->getRawCharCode();
						if (r && *r == '\n') {
							++index;
							++i;
						}
					}
				}
				else if (ch == '\n') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
				}
				else {
					++rowCol.second;
					++index;
					++i;
				}
				if (pRowColLastNonNull != NULL) {
					*pRowColLastNonNull = rowCol;
				}
				if (pIndexLastNonNull != NULL) {
					*pIndexLastNonNull = index;
				}
				continue;
			}
			const text::GeneratedToken *g = p->castToGenerated();
			if (g != NULL) {
				const TokenSequence &value = g->value;
				if (g->code != text::GeneratedToken::cNULL) {
					print_i_silent(&rowCol, pRowColLastNonNull, &index, pIndexLastNonNull, value);
				}
				else {
					print_i_silent(&rowCol, NULL, &index, NULL, value);
				}
				++i;
				continue;
			}
		}
	}
	static void print_i_leaftext(std:: pair<size_t/* row */, size_t/* col */> *pRowCol, size_t *pIndex, 
			const TokenSequence &text, const PrintData &data)	
	{
		std:: pair<size_t/* row */, size_t/* col */> &rowCol = *pRowCol;
		size_t &index = *pIndex;
		std:: ostream &output = *data.pOutput;

		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *p = text.refAt(i);
			if (p == NULL) {
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = p->getRawCharCode();
			if (r) {
				MYWCHAR_T ch = *r;
				write(&output, data.pRawCharEncoder->encode(ch));
				if (ch == '\r') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
					if (i < text.size()) {
						r = text.refAt(i)->getRawCharCode();
						if (r && (ch = *r) == '\n') {
							write(&output, data.pRawCharEncoder->encode(ch));
							++index;
							++i;
						}
					}
				}
				else if (ch == '\n') {
					++rowCol.first;
					rowCol.second = 1;
					++index;
					++i;
				}
				else {
					++rowCol.second;
					++index;
					++i;
				}
				continue;
			}
			const text::GeneratedToken *g = p->castToGenerated();
			if (g != NULL) {
				const TokenSequence &value = g->value;
				if (g->code == 0) {
					print_i_silent(&rowCol, &index, value);
				}
				else {
					print_i_leaftext(&rowCol, &index, value, data);
				}
				++i;
				continue;
			}
		}
	}
	static void printCng_i_leaftext(std:: ostream *pOutput, const TokenSequence &text)
	{
		std:: ostream &output = *pOutput;

		size_t i = 0; 
		while (i < text.size()) {
			const text::Token *p = text.refAt(i);
			if (p == NULL) {
				++i;
				continue;
			}
			boost::optional<MYWCHAR_T> r = p->getRawCharCode();
			if (r) {
				MYWCHAR_T ch = *r;
				output << common::EscapeSequenceHelper::encode(ch, false);
				if (ch == '\r') {
					++i;
					if (i < text.size()) {
						r = text.refAt(i)->getRawCharCode();
						if (r && (ch = *r) == '\n') {
							output << common::EscapeSequenceHelper::encode(ch, false);
							++i;
						}
					}
				}
				else if (ch == '\n') {
					++i;
				}
				else {
					++i;
				}
				continue;
			}
			const text::GeneratedToken *g = p->castToGenerated();
			if (g != NULL) {
				const TokenSequence &value = g->value;
				if (g->code == 0) {
					NULL;
				}
				else {
					printCng_i_leaftext(&output, value);
				}
				++i;
				continue;
			}
		}
	}
};

}; // namespace

#endif // Token_H
