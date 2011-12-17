#include <vector>

#include <boost/pool/object_pool.hpp>

#include "texttoken.h"

const boost::int32_t/* code */ text::GeneratedToken::cNULL = 0;

namespace {

std:: vector<std:: pair<std:: vector<MYWCHAR_T>/* name */, text::GeneratedToken *> > make_special_tokens()
{
	const char *specials[] = { "null", "any", "eof", "eol", "raw", NULL };

	std:: vector<std:: pair<std:: vector<MYWCHAR_T>/* name */, text::GeneratedToken *> > tokens;
	for (size_t i = 0; specials[i] != NULL; ++i) {
		std:: vector<MYWCHAR_T> t;
		common::EscapeSequenceHelper::decode(&t, specials[i]);
		tokens.push_back(std:: pair<std:: vector<MYWCHAR_T>/* name */, text::GeneratedToken *>(
			t, text::GeneratedToken::create((boost::int32_t)i, text::TokenSequence())));
	}

	return tokens;
}

}; // namespace

const std:: vector<std:: pair<std:: vector<MYWCHAR_T>/* name */, text::GeneratedToken *> > text::GeneratedToken::SpecialTokens
		 = make_special_tokens();

namespace text {

#if defined CHECK_POS_CONTINUOUS

struct ConcreteRawCharToken : public RawCharToken {
protected:
	MYWCHAR_T code;
	boost::int32_t pos;
protected:
	ConcreteRawCharToken(MYWCHAR_T code_, boost::int32_t pos_)
		: code(code_), pos(pos_)
	{
	}
public:
	Token *dup() const
	{
		return new ConcreteRawCharToken(this->code, this->pos);
	}
	boost::int32_t getPos() const
	{
		return pos;
	}
	bool operator==(const Token &right) const
	{
		const ConcreteRawCharToken *pRight = right.castRawChar();
		if (pRight != NULL) {
			if (this->code == pRight->getCode() && this->pos == pRight->getPos()) {
				return true;
			}
		}

		return false;
	}
	virtual boost::optional<MYWCHAR_T> getRawCharCode() const 
	{
		return boost::optional<MYWCHAR_T>(code);
	}
	virtual void destroy()
	{
		delete this;
	}
};

RawCharToken *RawCharToken::create(MYWCHAR_T code_, boost::int32_t pos_)
{
	return new ConcreteRawCharToken(code_, pos_);
}

#else // CHECK_POS_CONTINUOUS

struct UnpooledRawCharToken : public RawCharToken {
	friend struct RawCharToken;

#if defined USE_BOOST_POOL
protected:
	static boost::object_pool<UnpooledRawCharToken> ThePool;
#endif

protected:
	MYWCHAR_T code;

public:
	UnpooledRawCharToken()
		: code(0)
	{
	}
protected:
	UnpooledRawCharToken(MYWCHAR_T code_)
		: code(code_)
	{
	}
public:
	virtual Token *dup() const
	{
#if defined USE_BOOST_POOL
		UnpooledRawCharToken *p = UnpooledRawCharToken::ThePool.construct();
		p->code = code;
		return p;
#else
		return new UnpooledRawCharToken(this->code);
#endif
	}
	void setCode(MYWCHAR_T code_)
	{
		code = code_;
	}

public:
	virtual bool operator==(const Token &right) const
	{
		boost::optional<MYWCHAR_T> r = right.getRawCharCode();
		if (r && this->code == *r) {
            return true;
		}

		return false;
	}
	virtual boost::optional<MYWCHAR_T> getRawCharCode() const
	{
		return boost::optional<MYWCHAR_T>(code);
	}
	virtual void destroy()
	{
#if defined USE_BOOST_POOL
		UnpooledRawCharToken::ThePool.destroy(this);
#else
		delete this;
#endif
	}
};

RawCharToken *RawCharToken::create(MYWCHAR_T code_, boost::int32_t pos_)
{
#if defined USE_BOOST_POOL
	UnpooledRawCharToken *p = UnpooledRawCharToken::ThePool.construct();
	p->setCode(code_);
	return p;
#else
	return new UnpooledRawCharToken(code_);
#endif
}

#endif // CHECK_POS_CONTINUOUS

}; // namespace

#if defined USE_BOOST_POOL
boost::object_pool<text::UnpooledRawCharToken> text::UnpooledRawCharToken::ThePool;
#endif

#if defined USE_BOOST_POOL
boost::object_pool<text::GeneratedToken> text::GeneratedToken::ThePool;
#endif



