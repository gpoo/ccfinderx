#if ! defined INTERPRETER_H
#define INTERPRETER_H

#include <vector>
#include <cassert>
#include <map>
#include <set>

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "../common/utf8support.h" 
#include "texttoken.h"
#include "torqparser.h"

class LabelCodeTableBase {
private:
	std:: map<std:: vector<MYWCHAR_T>, boost::int32_t> labelNameToIndexTable;
	std:: vector<std:: vector<MYWCHAR_T> > labelIndexToNameTable;

public:
	virtual ~LabelCodeTableBase() { }
	LabelCodeTableBase()
	{
		const std:: vector<std:: pair<std:: vector<MYWCHAR_T>/* name */, text::GeneratedToken *> > &specialTokens
				= text::GeneratedToken::SpecialTokens;
		
		for (size_t i = 0; i < specialTokens.size(); ++i) {
			labelIndexToNameTable.push_back(specialTokens[i].first);
			labelNameToIndexTable[specialTokens[i].first] = (boost::int32_t)i;
		}
	}
public:
	boost::int32_t allocLabelCode(const std:: vector<MYWCHAR_T> &label)
	{
		boost::int32_t labelCode = -1;
		std:: map<std:: vector<MYWCHAR_T>, boost::int32_t>::iterator li = labelNameToIndexTable.find(label);
		if (li == labelNameToIndexTable.end()) {
			labelCode = labelNameToIndexTable.size();
			labelNameToIndexTable[label] = labelCode;
			labelIndexToNameTable.push_back(label);
		}
		else {
			labelCode = li->second;
		}
		return labelCode;
	}
public:
	size_t getNumberOfLabels() const
	{
		return labelIndexToNameTable.size();
	}
	std:: vector<MYWCHAR_T> getLabelString(boost::int32_t labelCode) const
	{
		size_t size = getNumberOfLabels();
		assert(0 <= labelCode && labelCode < size);
		return labelIndexToNameTable[labelCode];
	}
	std:: vector<std:: vector<MYWCHAR_T> > getLabelStrings() const
	{
		return labelIndexToNameTable;
	}
};

class LabelCodeTableSingleton : public LabelCodeTableBase {
private:
	static boost::shared_ptr<LabelCodeTableBase> pTheInstance;
public:
	static boost::shared_ptr<LabelCodeTableBase> instance()
	{
		if (pTheInstance.get() == NULL) {
			boost::shared_ptr<LabelCodeTableBase> p(new LabelCodeTableSingleton);
			pTheInstance.swap(p);
		}
		return pTheInstance;
	}
private:
	LabelCodeTableSingleton() { }
};


class Interpreter {
protected:
	struct MATCH {
	public:
		enum Classification {
			M_NUL, M_COPY, M_PACKBEGIN, M_PACKEND, M_CHILDRENBEGIN, M_CHILDRENEND
		};
	private:
		boost::int32_t value;
		/*
		>= 0 : copy
		-1 : packEnd
		-2 : childrenBegin
		-3 : childrenEnd
		<= -4 : packBegin
		*/
	public:
		void swap(MATCH &right)
		{
			std:: swap(value, right.value);
		}
	public:
		size_t size() const
		{
			assert(value >= 0);
			return value;
		}
		boost::int32_t getCode() const
		{
			assert(value <= -4);
			boost::int32_t code = -(value + 4);
			assert(code >= 0);
			return code;
		}
		void extend(size_t extendingLength)
		{
			assert(value >= 0);
			value += extendingLength;
		}
	public:
		MATCH &assignCopy(const text::TokenSequence *pSource_, boost::int32_t begin_, boost::int32_t end_)
		{
			assert(pSource_ != NULL);
			assert(0 <= begin_);
			assert(begin_ <= end_);
			assert(end_ <= (*pSource_).size());
			value = end_ - begin_;
			return *this;
		}
		static MATCH makeCopy(const text::TokenSequence *pSource, boost::int32_t begin, boost::int32_t end)
		{
			return MATCH().assignCopy(pSource, begin, end);
		}
		MATCH &assignCopy1(const text::TokenSequence *pSource_, boost::int32_t begin_)
		{
			assert(pSource_ != NULL);
			assert(0 <= begin_);
			assert(begin_ + 1 <= (*pSource_).size());
			value = 1;
			return *this;
		}
		static MATCH makeCopy1(const text::TokenSequence *pSource, boost::int32_t begin)
		{
			return MATCH().assignCopy1(pSource, begin);
		}
		MATCH &assignPackBegin(boost::int32_t code)
		{
			assert(code >= 0);
			value = -(code + 4);
			return *this;
		}
		static MATCH makePackBegin(boost::int32_t code)
		{
			return MATCH().assignPackBegin(code);
		}
		MATCH &assignPackEnd()
		{
			value = -1;
			return *this;
		}
		static MATCH makePackEnd()
		{
			return MATCH().assignPackEnd();
		}
		MATCH &assignChildrenBegin()
		{
			value = -2;
			return *this;
		}
		static MATCH makeChildrenBegin()
		{
			return MATCH().assignChildrenBegin();
		}
		MATCH &assignChildrenEnd()
		{
			value = -3;
			return *this;
		}
		static MATCH makeChildrenEnd()
		{
			return MATCH().assignChildrenEnd();
		}
		bool isCopy() const
		{
			return value >= 0;
		}
		bool isPackEnd() const 
		{
			return value == -1;
		}
		bool isPackBegin() const 
		{
			return value <= -4;
		}
		bool isChildrenBegin() const
		{
			return value == -2;
		}
		bool isChildrenEnd() const
		{
			return value == -3;
		}
		bool isSomethingEnd() const
		{
			return value == -1 || value == -3;
		}
		Classification getClassification() const
		{
			switch (value) {
			case -1:
				return M_PACKEND;
			case -2:
				return M_CHILDRENBEGIN;
			case -3:
				return M_CHILDRENEND;
			default:
				if (value >= 0) {
					return M_COPY;
				}
				else {
					return M_PACKBEGIN;
				}
			}
		}
	};

	static void expandMatchSeqOutOfPack(text::TokenSequence *pResult, text::TokenSequence *pSource,
			std:: vector<MATCH>::const_iterator *pMi, const std:: vector<MATCH>::const_iterator &miEnd)
	{
		std:: vector<MATCH>::const_iterator &mi = *pMi;
		text::TokenSequence &result = *pResult;
		text::TokenSequence &source = *pSource;
		size_t si = 0;
		size_t siEnd = source.size();
		
		for (std:: vector<MATCH>::const_iterator miBegin = mi; mi != miEnd && ! mi->isSomethingEnd(); ++mi) {
			const MATCH &m = *mi;
			switch (m.getClassification()) {
			case MATCH::M_COPY:
				{
					si += m.size();
				}
				break;
			case MATCH::M_CHILDRENBEGIN:
				{
					++mi;
					text::GeneratedToken *pGen = source.refAt(si)->castToGenerated();
					assert(pGen != NULL);
					text::TokenSequence childrenResult;
					expandMatchSeqOutOfPack(&childrenResult, &pGen->value, &mi, miEnd);
					assert(mi != miEnd);
					assert((*mi).isChildrenEnd());
					pGen->value.swap(childrenResult);
					++si;
				}
				break;
			case MATCH::M_PACKBEGIN:
				{
					for (size_t sj = 0; sj < si; ++sj) {
						text::Token *p = source.refAt(sj);
						assert(p != NULL);
						source.replaceAt(sj, NULL);
						result.attachBack(p);
					}
					expandMatchSeq(pResult, pSource, &si, &mi, miEnd);
					assert(si == siEnd);
					return;
				}
				break;
			default:
				assert(false);
				return;
			}
		}
		result.swap(source);
		assert(si == siEnd);
	}
	static void expandMatchSeq(text::TokenSequence *pResult, text::TokenSequence *pSource, size_t *pSourceIndex,
			std:: vector<MATCH>::const_iterator *pMi, const std:: vector<MATCH>::const_iterator &miEnd)
	{
		std:: vector<MATCH>::const_iterator &mi = *pMi;
		text::TokenSequence &result = *pResult;
		text::TokenSequence &source = *pSource;
		size_t &sourceIndex = *pSourceIndex;

		while (mi != miEnd && ! mi->isSomethingEnd()) {
			const MATCH &m = *mi;
			switch (m.getClassification()) {
			case MATCH::M_COPY:
				{
					size_t length = m.size();
					++mi;
					while (mi != miEnd && mi->isCopy()) {
						length += mi->size();
						++mi;
					}
					if (mi == miEnd || mi->isSomethingEnd()) {
						result.reserve(result.size() + length);
					}
					for (size_t i = 0; i < length; ++i) {
						text::Token *p = source.refAt(sourceIndex);
						assert(p != NULL);
						boost::optional<boost::int32_t> r;
						bool nullMerging = (r = p->getGeneratedCode()) && *r == cNULL 
								&& result.size() > 0  && (r = result.refBack()->getGeneratedCode()) && *r == cNULL;
						source.replaceAt(sourceIndex, NULL);
						if (nullMerging) {
							text::GeneratedToken *pGen = result.refBack()->castToGenerated();
							text::GeneratedToken *pSrc = p->castToGenerated();
							for (size_t j = 0; j < pSrc->value.size(); ++j) {
								text::Token *q = pSrc->value.replaceAt(j, NULL);
								pGen->value.attachBack(q);
							}
							assert(p->isGenerated());
							p->destroy();
						}
						else {
							result.attachBack(p);
						}
						++sourceIndex;
					}
				}
				break;
			case MATCH::M_CHILDRENBEGIN:
				{
					++mi;
					text::GeneratedToken *pGen = source.refAt(sourceIndex)->castToGenerated();
					assert(pGen != NULL);
					text::TokenSequence childrenResult;
					expandMatchSeqOutOfPack(&childrenResult, &pGen->value, &mi, miEnd);
					assert(mi != miEnd);
					assert((*mi).isChildrenEnd());
					++mi;
					pGen->value.swap(childrenResult);
					source.replaceAt(sourceIndex, NULL);
					result.attachBack(pGen);
					++sourceIndex;
				}
				break;
			case MATCH::M_PACKBEGIN:
				{
					assert(m.getCode() >= 0);
					boost::optional<boost::int32_t> r;
					bool nullMerging = m.getCode() == cNULL && result.size() > 0 
						&& (r = result.refBack()->getGeneratedCode()) && *r == cNULL;
					text::GeneratedToken *pGenerated = NULL;
					if (nullMerging) {
						pGenerated = result.refBack()->castToGenerated();
					}
					else {
						pGenerated = text::GeneratedToken::create();
						pGenerated->code = m.getCode();
					}
					++mi;
					expandMatchSeq(&pGenerated->value, pSource, pSourceIndex, &mi, miEnd);
					assert(mi != miEnd);
					assert((*mi).isPackEnd());
					++mi;
					if (! nullMerging) {
						result.attachBack(pGenerated);
					}
				}
				break;
			default:
				assert(false);
				return;
			}
		}
		return;
	}

	static void expandMatchSeq(text::TokenSequence *pResult, text::TokenSequence *pSource, const std:: vector<MATCH> &matchSeq)
	{
		std:: vector<MATCH>::const_iterator mi = matchSeq.begin();
		expandMatchSeqOutOfPack(pResult, pSource, &mi, matchSeq.end());
		assert(mi == matchSeq.end());
	}

public:
	static std:: vector<TRACE_ITEM> remove_unrequired_items(const std:: vector<TRACE_ITEM> &trace)
	{
		TraceDirector td;
		td.setTrace(trace);
		
		std:: set<boost::int32_t> unrequired;
		{
			for (boost::int32_t i = 0; i < trace.size(); ++i) {
				const TRACE_ITEM &item = trace[i];
				if (item.classification == TRACE_ITEM::Enter) {
					switch (item.node) {
					case NC_AtomPattern:
						unrequired.insert(i);
						break;
					case NC_ParenPattern:
						unrequired.insert(i);
						break;
					case NC_Pattern:
						if (i - 1 >= 0 && trace[i - 1].classification == TRACE_ITEM::Enter && trace[i - 1].node == NC_ParenPattern) {
							unrequired.insert(i);
						}
						break;
					case NC_Statement:
						unrequired.insert(i);
						break;
					default:
						break;
					}
				}
				else if (item.classification == TRACE_ITEM::Accept) {
					switch (item.node) {
					case NC_OrPattern:
						{
							bool hasPrev = i + 1 < trace.size() && trace[i + 1].classification == TRACE_ITEM::Accept && trace[i + 1].node == NC_OrPattern;
							if (! hasPrev) {
								bool hasNext = i - 1 >= 0 && trace[i - 1].classification == TRACE_ITEM::Accept && trace[i - 1].node == NC_OrPattern;
								if (! hasNext) {
									unrequired.insert(i);
								}
							}
						}
						break;
					case NC_PackPattern:
					case NC_MatchPattern:
					case NC_ScanPattern:
						assert(item.ref.classification != TOKEN::NUL);
						break;
					case NC_RepeatPattern:
						if (item.ref.classification == TOKEN::NUL) {
							unrequired.insert(i);
						}
						break;
					case NC_SequencePattern:
						{
							bool hasPrev = i + 1 < trace.size() && trace[i + 1].classification == TRACE_ITEM::Accept && trace[i + 1].node == NC_SequencePattern;
							if (! hasPrev) {
								bool hasNext = i - 1 >= 0 && trace[i - 1].classification == TRACE_ITEM::Accept && trace[i - 1].node == NC_SequencePattern;
								if (! hasNext) {
									unrequired.insert(i);
								}
							}
						}
						break;
					default:
						break;
					}
				}
				else {
					assert(false); // when the trace includes Reject (not implemented yet)
				}
			}
			{
				std:: set<boost::int32_t> additionals;
				for (std:: set<boost::int32_t>::const_iterator ui = unrequired.begin(); ui != unrequired.end(); ++ui) {
					boost::int32_t idx = *ui;
					boost::int32_t pairIdx = td.findPair(idx);
					additionals.insert(pairIdx);		
				}
				unrequired.insert(additionals.begin(), additionals.end());
			}
		}
		std:: vector<TRACE_ITEM> shapeupTrace;
		for (boost::int32_t i = 0; i < trace.size(); ++i) {
			if (unrequired.find(i) == unrequired.end()) {
				shapeupTrace.push_back(trace[i]);
			}
		}
		return shapeupTrace;
	}

public:
	enum ErrorCode {
		Nul = 0, InvalidStartPC, UndefinedVariable, Unmatch, Cutoff
	};
	struct Error {
	public:
		ErrorCode code;
		std:: vector<MYWCHAR_T> ref;
	public:
		Error()
			: code(Nul), ref()
		{
		}
		Error(const Error &right)
			: code(right.code), ref(right.ref)
		{
		}
		Error(ErrorCode code_, const std:: vector<MYWCHAR_T> &ref_)
			: code(code_), ref(ref_)
		{
		}
		Error(ErrorCode code_)
			: code(code_), ref()
		{
		}
	public:
		void swap(Error &right)
		{
			std:: swap(this->code, right.code);
			ref.swap(right.ref);
		}
	};
protected:
	struct TokenStringTableItem {
	public:
		bool isCharClass;
		std:: vector<MYWCHAR_T> str;
		std:: pair<MYWCHAR_T, MYWCHAR_T> range;
	public:
		void swap(TokenStringTableItem &right)
		{
			std:: swap(this->isCharClass, right.isCharClass);
			this->str.swap(right.str);
#if defined __GNUC__
			std:: swap(this->range, right.range);
#else
			this->range.swap(right.range);
#endif
		}
	};
	struct TSL {
	public:
		TRACE_ITEM item;
		TokenStringTableItem str;
		boost::int32_t label;
		boost::int32_t next;
	public:
		TSL()
			: item(), str(), label(0), next(0)
		{
		}
		TSL(const TSL &right)
			: item(right.item), str(right.str), label(right.label), next(right.next)
		{
		}
		TSL(const TRACE_ITEM &item_, const TokenStringTableItem &str_, boost::int32_t label_, boost::int32_t next_)
			: item(item_), str(str_), label(label_), next(next_)
		{
		}
	};
protected:
	std:: vector<MYWCHAR_T> script;
	std:: vector<TRACE_ITEM> programv;
	TraceDirector td;
	std:: map<std:: vector<MYWCHAR_T>, text::TokenSequence> variables;
	boost::int32_t errorPc;
	long long cutoffValue;
	long long cutoffTimer;
	Error errorData;
	std:: vector<boost::int32_t> recursePc;
	std:: vector<TSL> tdata;
	std:: vector<MATCH> matchSeq;
	static const boost::int32_t/* code */ cNULL;
	static const boost::int32_t/* code */ cANY;
	static const boost::int32_t/* code */ cEOF;
	static const boost::int32_t/* code */ cEOL;
	static const boost::int32_t/* code */ cRAW;
	boost::shared_ptr<LabelCodeTableBase> pLabelCodeTable;
public:
	~Interpreter()
	{
	}
	Interpreter()
		: cutoffValue(0), pLabelCodeTable(LabelCodeTableSingleton::instance())
	{
	}
protected:
	boost::int32_t allocLabelCode(boost::int32_t pc)
	{
		const TRACE_ITEM &citem = programv[pc];
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.ref.classification != TOKEN::NUL);
		std:: vector<MYWCHAR_T> label;
		label.insert(label.end(), script.begin() + citem.ref.beginPos, script.begin() + citem.ref.endPos);
		return pLabelCodeTable->allocLabelCode(label);
	}
public:
	void setProgram(const std:: vector<TRACE_ITEM> &program_, const std:: vector<MYWCHAR_T> &script_)
	{
		programv = remove_unrequired_items(program_);
		script = script_;
		td.setTrace(programv);

		tdata.resize(programv.size());

		{
			for (boost::int32_t pc = 0; pc < programv.size(); ++pc) {
				tdata[pc].item = programv[pc];
				tdata[pc].next = td.findNext(pc);
			}
		}
		
		// setup label table
		{
			std:: set<std:: vector<MYWCHAR_T> > labels;
			for (boost::int32_t pc = 0; pc < tdata.size(); ++pc) {
				const TRACE_ITEM &citem = tdata[pc].item;
				if (citem.classification == TRACE_ITEM::Accept) {
					switch (citem.node) {
					case NC_PackPattern:
					case NC_MatchPattern:
					case NC_ScanPattern:
					case NC_GeneratedTokenPattern:
					case NC_InsertPattern:
						{
							assert(citem.ref.classification != TOKEN::NUL);
							std:: vector<MYWCHAR_T> label;
							label.insert(label.end(), script.begin() + citem.ref.beginPos, script.begin() + citem.ref.endPos);
							labels.insert(label);
						}
						break;
					default:
						break;
					}
				}
			}
			for (std:: set<std:: vector<MYWCHAR_T> >::const_iterator i = labels.begin(); i != labels.end(); ++i) {
				const std:: vector<MYWCHAR_T> &label = *i;
				pLabelCodeTable->allocLabelCode(label);
			}
			for (boost::int32_t pc = 0; pc < tdata.size(); ++pc) {
				const TRACE_ITEM &citem = tdata[pc].item;
				if (citem.classification == TRACE_ITEM::Accept) {
					switch (citem.node) {
					case NC_PackPattern:
					case NC_MatchPattern:
					case NC_ScanPattern:
					case NC_GeneratedTokenPattern:
					case NC_InsertPattern:
						{
							boost::int32_t labelCode = allocLabelCode(pc);
							tdata[pc].label = labelCode;
							
							boost::int32_t p = td.findPair(pc);
							tdata[p].label = labelCode;
						}
						break;
					default:
						break;
					}
				}
			}
		}

		// setup string table
		{
			for (boost::int32_t pc = 0; pc < tdata.size(); ++pc) {
				const TRACE_ITEM &citem = tdata[pc].item;
				if (citem.classification == TRACE_ITEM::Accept) {
					if (citem.node == NC_LiteralPattern) {
						assert(citem.classification == TRACE_ITEM::Accept);
						assert(citem.node == NC_LiteralPattern);
						assert(citem.ref.classification != TOKEN::NUL);
						
						const TOKEN &t0 = citem.ref;
						TOKEN t(t0);
						t.beginPos += 1;
						t.endPos -= 1;
						assert(t.beginPos <= t.endPos);
						std:: vector<MYWCHAR_T> token;
						common::EscapeSequenceHelper::decode(&token, script, t.beginPos, t.endPos);
						TokenStringTableItem ci;
						bool isCharClass = false;
						if (token.size() == 7 && token[0] == '&' && token[1] == '(' && token[3] == '-' && token[5] == ')' && token[6] == ';') {
							MYWCHAR_T fromC = token[2];
							MYWCHAR_T toC = token[4];
							ci.isCharClass = true;
							ci.range.first = fromC;
							ci.range.second = toC;
							isCharClass = true;
						}
						if (! isCharClass) {
							ci.isCharClass = false;
							ci.str.swap(token);
						}
						tdata[pc].str = ci;

						boost::int32_t p = td.findPair(pc);
						tdata[p].str = ci;
					}
					else if (citem.node == NC_RepeatPattern) {
						assert(citem.classification == TRACE_ITEM::Accept);
						assert(citem.node == NC_RepeatPattern);
						assert(citem.ref.classification != TOKEN::NUL);
						
						assert(citem.ref.endPos - citem.ref.beginPos == 1);
						TokenStringTableItem ci;
						MYWCHAR_T markChar = script[citem.ref.beginPos];
						switch (markChar) {
						case '?':
							ci.range.first = 0;
							ci.range.second = 1;
							break;
						case '+':
							ci.range.first = 1;
							ci.range.second = 999;
							break;
						case '*':
							ci.range.first = 0;
							ci.range.second = 999;
						}
						
						tdata[pc].str = ci;

						boost::int32_t p = td.findPair(pc);
						tdata[p].str = ci;
					}
				}
			}
		}
		
		// copy the reference of items to their pair items
		{
			for (boost::int32_t pc = 0; pc < tdata.size(); ++pc) {
				const TRACE_ITEM &citem = tdata[pc].item;
				if (citem.classification == TRACE_ITEM::Accept) {
					boost::int32_t p = td.findPair(pc);
					assert(p != -1);
					TRACE_ITEM &item = tdata[p].item;
					assert(item.ref.classification == TOKEN::NUL);
					item.ref = citem.ref;
				}
			}
		}
	}
	std:: vector<TRACE_ITEM> getProgram() const
	{
		return programv;
	}
	void swapVariable(const std:: vector<MYWCHAR_T> &name, text::TokenSequence *pValue)
	{
		variables[name].swap(*pValue);
	}
	size_t getNumberOfLabels() const
	{
		return pLabelCodeTable->getNumberOfLabels();
	}
	std:: vector<MYWCHAR_T> getLabelString(boost::int32_t labelCode) const
	{
		return pLabelCodeTable->getLabelString(labelCode);
	}
	std:: vector<std:: vector<MYWCHAR_T> > getLabelStrings() const
	{
		return pLabelCodeTable->getLabelStrings();
	}
	void setCutoffValue(long long cutoffValue_) 
	{
		this->cutoffValue = cutoffValue_;
	}
	Error getError() const
	{
		return errorData;
	}
	boost::int32_t interpret(boost::int32_t pcStart)
	{
		errorPc = -1; // clear
		errorData = Error(Nul);
		assert(0 <= pcStart && pcStart <= programv.size());
		boost::int32_t pc = pcStart;
		const TRACE_ITEM &item = tdata[pc].item;
		if (item.classification != TRACE_ITEM::Enter) {
			errorData = Error(InvalidStartPC);
			errorPc = pc; // invalid start pc
			return errorPc;
		}
		try {
			switch (item.node) {
			case NC_Statements:
				do_Statements(pc);
				break;
			case NC_AssignStatement:
				do_AssignStatement(pc);
				break;
			case NC_ScanEqStatement:
				do_ScanEqStatement(pc);
				break;
			case NC_MatchEqStatement:
				do_MatchEqStatement(pc);
				break;
			default:
				errorData = Error(InvalidStartPC);
				errorPc = pc; // invalid start pc
				return errorPc;
			}
		}
		catch (Error &) {
			return errorPc;
		}
		return -1; // no error
	}
protected:
	void do_Statements(boost::int32_t pc0)
	{
		boost::int32_t pc = pc0;
		++pc;
		while (pc != -1) {
			interpret(pc);
			if (errorPc != -1) {
				throw errorData;
			}
			pc = tdata[pc].next;
		}
	}
	void do_AssignStatement(boost::int32_t pc0)
	{
		assert(false); // not implemented yet
	}
	static void skip_null(const text::TokenSequence &source, boost::int32_t *pPos)
	{
		boost::int32_t &pos = *pPos;
		assert(0 <= pos && pos <= source.size());
		while (pos < source.size()) {
			assert(source.refAt(pos) != NULL);
			boost::optional<boost::int32_t> r;
			if (! (r = source.refAt(pos)->getGeneratedCode()) || *r != cNULL) {
				break;
			}
			++pos;
		}
	}
	void extendNullMatchSeq(const text::TokenSequence &text, boost::int32_t *pPos)
	{
		boost::int32_t pos0 = *pPos;
		boost::int32_t pos = pos0;
		skip_null(text, &pos);
		if (pos - pos0 > 0) {
			matchSeq.push_back(MATCH::makeCopy(&text, pos0, pos));
		}
		*pPos = pos;
	}
	void extend1MatchSeq(size_t bottomSize, const text::TokenSequence &text, boost::int32_t pos)
	{
		MATCH *pLastMatch;
		if (matchSeq.size() > bottomSize && (pLastMatch = &matchSeq.back())->isCopy()) {
			pLastMatch->extend(1);
		}
		else {
			matchSeq.push_back(MATCH::makeCopy1(&text, pos));
		}
	}
	void do_ScanEqStatement(boost::int32_t pc0)
	{
		const TRACE_ITEM &item0 = tdata[pc0].item;
#if ! defined NDEBUG
		{
			boost::int32_t p = td.findPair(pc0);
			const TRACE_ITEM &citem = tdata[p].item;
			assert(citem.classification == TRACE_ITEM::Accept);
			assert(citem.node == NC_ScanEqStatement);
			assert(citem.ref.classification != TOKEN::NUL);
			assert(citem.ref == item0.ref);
		}
#endif
		
		std:: vector<MYWCHAR_T> varName;
		varName.insert(varName.end(), script.begin() + item0.ref.beginPos, script.begin() + item0.ref.endPos);
		std:: map<std:: vector<MYWCHAR_T>, text::TokenSequence>::iterator i = variables.find(varName);
		if (i == variables.end()) {
			errorData = Error(UndefinedVariable, varName);
			errorPc = pc0;
			throw errorData;
		}
		text::TokenSequence &varValue = i->second;

		boost::int32_t pc = pc0 + 1;
		const TRACE_ITEM &item = tdata[pc].item;
		assert(item.classification == TRACE_ITEM::Enter);
		assert(item.node == NC_Pattern);
		boost::int32_t pos = 0;
		matchSeq.resize(0);
		if (matchSeq.capacity() < varValue.size() * 3) {
			matchSeq.reserve(varValue.size() * 3);
		}

		extendNullMatchSeq(varValue, &pos);
		if (cutoffValue != 0) {
			cutoffTimer = cutoffValue;
		}
		while (pos < varValue.size()) {
			recursePc.resize(0);
			boost::int32_t q = eval(pc, varValue, pos);
			assert(errorPc == -1);
			if (q <= pos) {
				if (cutoffValue != 0 && cutoffTimer < 0) {
					errorData = Error(Cutoff);
					errorPc = pc0;
					throw errorData;
				}
				assert(q < 0 || q == pos);
				extend1MatchSeq(0, varValue, pos);
				++pos;
			}
			else {
				pos = q;
			}
			extendNullMatchSeq(varValue, &pos);
		}

		text::TokenSequence result;
		expandMatchSeq(&result, &varValue, matchSeq);
		varValue.swap(result);
	}
	void do_MatchEqStatement(boost::int32_t pc0)
	{
		const TRACE_ITEM &item0 = tdata[pc0].item;
#if ! defined NDEBUG
		{
			boost::int32_t p = td.findPair(pc0);
			const TRACE_ITEM &citem = tdata[p].item;
			assert(citem.classification == TRACE_ITEM::Accept);
			assert(citem.node == NC_MatchEqStatement);
			assert(citem.ref.classification != TOKEN::NUL);
			assert(citem.ref == item0.ref);
		}
#endif

		std:: vector<MYWCHAR_T> varName;
		varName.insert(varName.end(), script.begin() + item0.ref.beginPos, script.begin() + item0.ref.endPos);
		std:: map<std:: vector<MYWCHAR_T>, text::TokenSequence>::iterator i = variables.find(varName);
		if (i == variables.end()) {
			errorData = Error(UndefinedVariable, varName);
			errorPc = pc0;
			throw errorData;
		}
		text::TokenSequence &varValue = i->second;
		
		boost::int32_t pc = pc0 + 1;
		const TRACE_ITEM &item = tdata[pc].item;
		assert(item.classification == TRACE_ITEM::Enter);
		assert(item.node == NC_Pattern);
		boost::int32_t pos = 0;
		recursePc.resize(0);
		matchSeq.resize(0);
		if (matchSeq.capacity() < varValue.size() * 3) {
			matchSeq.reserve(varValue.size() * 3);
		}
		extendNullMatchSeq(varValue, &pos);
		if (cutoffValue != 0) {
			cutoffTimer = cutoffValue;
		}
		boost::int32_t q = eval(pc, varValue, pos);
		assert(errorPc == -1);
		if (q < 0) {
			if (cutoffValue != 0 && cutoffTimer < 0) {
				errorData = Error(Cutoff);
				errorPc = pc0;
				throw errorData;
			}
		}
		else {
			pos = q;
			extendNullMatchSeq(varValue, &pos);
		}
		if (pos == varValue.size()) {
			text::TokenSequence result;
			expandMatchSeq(&result, &varValue, matchSeq);
			varValue.swap(result);
		}
		else {
			errorData = Error(Unmatch);
			errorPc = pc0;
			throw errorData;
		}
	}
	typedef boost::int32_t (Interpreter::*pfunc_t) (boost::int32_t, const text::TokenSequence &, boost::int32_t);
#if defined INTERPRETER_USE_DISPATCHTABLE
	static const pfunc_t dispatchTable[NC_SIZE];
#endif
	boost::int32_t eval(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();

		if (cutoffValue != 0) {
			if (--cutoffTimer < 0) {
				return -1;
			}
		}
		
		boost::int32_t pos = pos0;
		///extendNullMatchSeq(source, &pos);
		size_t size1 = pMatchSeq->size();

		boost::int32_t pc = pc0;
		const TRACE_ITEM &item = tdata[pc].item;
		assert(item.classification == TRACE_ITEM::Enter);
		assert(NC_Nul < item.node && item.node < NC_SIZE); 
#if defined INTERPRETER_USE_DISPATCHTABLE
		pfunc_t pfunc = dispatchTable[item.node]; 
		boost::int32_t npos = (this->*pfunc)(pc, source, pos);
#else
		pfunc_t pfunc = NULL;
		switch (item.node) {
		case NC_Nul: pfunc = &Interpreter::do_assertionError; break;
		case NC_Statements: pfunc = &Interpreter::do_assertionError; break;
		case NC_Statement: pfunc = &Interpreter::do_assertionError; break;
		case NC_AssignStatement: pfunc = &Interpreter::do_assertionError; break;
		case NC_ScanEqStatement: pfunc = &Interpreter::do_assertionError; break;
		case NC_MatchEqStatement: pfunc = &Interpreter::do_assertionError; break;
		case NC_Expression: pfunc = &Interpreter::do_assertionError; break;
		case NC_Pattern: pfunc = &Interpreter::do_Pattern; break;
		case NC_PackPattern: pfunc = &Interpreter::do_PackPattern; break;
		case NC_MatchPattern: pfunc = &Interpreter::do_MatchPattern; break;
		case NC_ScanPattern: pfunc = &Interpreter::do_ScanPattern; break;
		case NC_OrPattern: pfunc = &Interpreter::do_OrPattern; break;
		case NC_SequencePattern: pfunc = &Interpreter::do_SequencePattern; break;
		case NC_RepeatPattern: pfunc = &Interpreter::do_RepeatPattern; break;
		case NC_AtomPattern: pfunc = &Interpreter::do_assertionError; break;
		case NC_ParenPattern: pfunc = &Interpreter::do_assertionError; break;
		case NC_XcepPattern: pfunc = &Interpreter::do_XcepPattern; break;
		case NC_PreqPattern: pfunc = &Interpreter::do_PreqPattern; break;
		case NC_LiteralPattern: pfunc = &Interpreter::do_LiteralPattern; break;
		case NC_GeneratedTokenPattern: pfunc = &Interpreter::do_GeneratedTokenPattern; break;
		case NC_RecursePattern: pfunc = &Interpreter::do_RecursePattern; break;
		case NC_InsertPattern: pfunc = &Interpreter::do_InsertPattern; break;
		default: pfunc = &Interpreter::do_assertionError; break;
		}
		boost::int32_t npos = (this->*pfunc)(pc, source, pos);
#endif // INTEREPRETER_USE_DISPATCHTABLE
		assert(errorPc == -1);
		if (npos < 0) {
			pMatchSeq->resize(size0);
			return -1;
		}
		else if (npos == pos /* match length is zero */
				&& pMatchSeq->size() == size1 /* no token is generated */) {
			pMatchSeq->resize(size0);
			return pos0;
		}
		else {
			return npos;
		}
	}
	boost::int32_t do_assertionError(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		assert(false);
		errorPc = pc0;
		errorData = Error();
		throw errorData;
		return -1;
	}
	boost::int32_t do_Pattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		recursePc.push_back(pc0);

		boost::int32_t pc = pc0 + 1;
#if ! defined NDEBUG
		const TRACE_ITEM &item = tdata[pc].item;
		assert(item.classification == TRACE_ITEM::Enter);
#endif
		try {
			boost::int32_t pos = pos0;
			extendNullMatchSeq(source, &pos);
			pos = eval(pc, source, pos);
			recursePc.pop_back();
			return pos;
		}
		catch (Error &e) {
			recursePc.pop_back();
			throw e;
		}
	}
	boost::int32_t do_PackPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();

		boost::int32_t pc = pc0 + 1;
		const TRACE_ITEM &item = tdata[pc].item;
		assert(item.classification == TRACE_ITEM::Enter);
#if ! defined NDEBUG
		boost::int32_t p = td.findPair(pc0);
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_PackPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t labelCode = tdata[pc0].label;

		// An optimization: When a an operator "<-" has a string literal on its left side, perform its matching here.
		{
			const TRACE_ITEM &item = tdata[pc].item;
			if (item.classification == TRACE_ITEM::Enter && item.node == NC_LiteralPattern) {
				boost::int32_t p = pc + 1;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_LiteralPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				boost::int32_t npos = substrEqualWithCharClass(source, pos0, p);
				if (npos >= 0) {
					size_t i0 = (*pMatchSeq).size();
					(*pMatchSeq).resize(i0 + 3);
					(*pMatchSeq)[i0].assignPackBegin(labelCode);
					(*pMatchSeq)[i0 + 1].assignCopy(&source, pos0, npos);
					(*pMatchSeq)[i0 + 2].assignPackEnd();
					return npos;
				}
				else {
					return -1;
				}
			}
		}

		pMatchSeq->push_back(MATCH::makePackBegin(labelCode));
		
		boost::int32_t pos = pos0;
		extendNullMatchSeq(source, &pos);
		pos = eval(pc, source, pos);
		assert(errorPc == -1);
		if (pos < 0) {
			pMatchSeq->resize(size0);
			return pos;
		}
		pMatchSeq->push_back(MATCH::makePackEnd());
		return pos;
	}
	boost::int32_t do_MatchPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();
		
#if ! defined NDEBUG
		boost::int32_t p = td.findPair(pc0);
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_MatchPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t labelCode = tdata[pc0].label;

		if (labelCode == cNULL) {
			return -1;
		}
		else {
			if (pos0 < source.size()) {
				assert(source.refAt(pos0) != NULL);
				const text::GeneratedToken *p = source.refAt(pos0)->castToGenerated();
				if (p != NULL && (labelCode == cANY || p->code == labelCode)) {
					boost::int32_t pc = pc0 + 1;
					const TRACE_ITEM &item = tdata[pc].item;
					assert(item.classification == TRACE_ITEM::Enter);
					pMatchSeq->push_back(MATCH::makeChildrenBegin());
					boost::int32_t pos = 0;
					extendNullMatchSeq(p->value, &pos);
					pos = eval(pc, p->value, pos);
					assert(errorPc == -1);
					if (pos < 0) {
						pMatchSeq->resize(size0);
						return pos;
					}

					extendNullMatchSeq(p->value, &pos);

					if (pos != p->value.size()) {
						pMatchSeq->resize(size0);
						return -1;
					}

					sequence_optimization(size0 + 1);
					if (pMatchSeq->size() == size0 + 2) {
						assert((*pMatchSeq)[size0].isChildrenBegin());
						assert((*pMatchSeq)[size0 + 1].isCopy());
						pMatchSeq->resize(size0 + 1);
						(*pMatchSeq)[size0].assignCopy1(&source, pos0);
					}
					else {
						pMatchSeq->push_back(MATCH::makeChildrenEnd());
					}
					return pos0 + 1;
				}
			}
		}
		pMatchSeq->resize(size0);
		return -1;
	}
	boost::int32_t do_ScanPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();
		
#if ! defined NDEBUG
		boost::int32_t p = td.findPair(pc0);
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_ScanPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t labelCode = tdata[pc0].label;

		if (labelCode == cNULL) {
			return -1;
		}
		else {
			if (pos0 < source.size()) {
				assert(source.refAt(pos0) != NULL);
				const text::GeneratedToken *p = source.refAt(pos0)->castToGenerated();
				if (p != NULL && (labelCode == cANY || p->code == labelCode)) {
					boost::int32_t pc = pc0 + 1;
					const TRACE_ITEM &item = tdata[pc].item;
					assert(item.classification == TRACE_ITEM::Enter);
					pMatchSeq->push_back(MATCH::makeChildrenBegin());
					boost::int32_t pos = 0;
					extendNullMatchSeq(p->value, &pos);
					while (pos < p->value.size()) {
						boost::int32_t q = eval(pc, p->value, pos);
						assert(errorPc == -1);
						if (q <= pos) {
							assert(q < 0 || q == pos);
							pMatchSeq->push_back(MATCH::makeCopy1(&p->value, pos));
							++pos;
						}
						else {
							pos = q;
						}
						extendNullMatchSeq(p->value, &pos);
					}

					sequence_optimization(size0 + 1);
					if (pMatchSeq->size() == size0 + 2) {
						assert((*pMatchSeq)[size0].isChildrenBegin());
						assert((*pMatchSeq)[size0 + 1].isCopy());
						pMatchSeq->resize(size0 + 1);
						(*pMatchSeq)[size0].assignCopy1(&source, pos0);
					}
					else {
						pMatchSeq->push_back(MATCH::makeChildrenEnd());
					}
					return pos0 + 1;
				}
			}
		}
		pMatchSeq->resize(size0);
		return -1;
	}
	boost::int32_t do_OrPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		boost::int32_t pc = pc0 + 1;
		boost::int32_t pos = pos0;
		extendNullMatchSeq(source, &pos);
		while (true) {
			const TRACE_ITEM &item = tdata[pc].item;
			assert(item.classification == TRACE_ITEM::Enter);
			boost::int32_t npos = -1;
#if 1 // simple optimization
			if (item.node == NC_LiteralPattern) {
				boost::int32_t p = pc + 1;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_LiteralPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				npos = substrEqualWithCharClass(source, pos0, p);
				if (npos >= 0) {
					pMatchSeq->push_back(MATCH::makeCopy(&source, pos0, npos));
					return npos;
				}
			}
			else if (item.node == NC_PackPattern && tdata[pc + 1].item.node == NC_LiteralPattern) {
				boost::int32_t p = pc + 2;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_LiteralPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				npos = substrEqualWithCharClass(source, pos0, p);
				if (npos >= 0) {
					boost::int32_t labelCode = tdata[pc].label;
					size_t i0 = (*pMatchSeq).size();
					(*pMatchSeq).resize(i0 + 3);
					(*pMatchSeq)[i0].assignPackBegin(labelCode);
					(*pMatchSeq)[i0 + 1].assignCopy(&source, pos0, npos);
					(*pMatchSeq)[i0 + 2].assignPackEnd();
					return npos;
				}
			}
			else {
				npos = eval(pc, source, pos);
			}
#else
			npos = eval(pc, source, pos);
#endif
			assert(errorPc == -1);
			if (npos < 0) {
				boost::int32_t q = tdata[pc].next;
				if (q == -1) {
					return -1;
				}
				const TRACE_ITEM &nitem = tdata[q].item;
				if (! (nitem.classification == TRACE_ITEM::Enter && nitem.node == NC_OrPattern)) {
					return -1; // not match
				}
				pc = q + 1;
			}
			else {
				return npos;
			}
		}
	}
	boost::int32_t do_SequencePattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();
		
		boost::int32_t pc = pc0 + 1;
		boost::int32_t pos = pos0;
		while (true) {
#if ! defined NDEBUG
			const TRACE_ITEM &item = tdata[pc].item;
			assert(item.classification == TRACE_ITEM::Enter);
#endif
			extendNullMatchSeq(source, &pos);

			boost::int32_t npos = eval(pc, source, pos);
			assert(errorPc == -1);
			if (npos < 0) {
				pMatchSeq->resize(size0);
				return npos;
			}
			else {
				pos = npos;
				boost::int32_t q = tdata[pc].next;
				if (q == -1) {
					break;
				}
				const TRACE_ITEM &nitem = tdata[q].item;
				if (! (nitem.classification == TRACE_ITEM::Enter && nitem.node == NC_SequencePattern)) {
					break;
				}
				pc = q + 1;
			}
		}
		sequence_optimization(size0);
		return pos;
	}
	void sequence_optimization(size_t size0)
	{
		if (matchSeq.size() >= size0 + 2) {
			MATCH *pTailMatch = &matchSeq.back();
			if (pTailMatch->isCopy()) {
				while (true) {
					MATCH *pLastMatch = pTailMatch; --pLastMatch;
					if (pLastMatch <= &matchSeq[size0]) {
						break; // while
					}
					if (! pLastMatch->isCopy()) {
						break; // whlie
					}
					pLastMatch->extend(pTailMatch->size());
					matchSeq.pop_back();
					--pTailMatch;
				}
			}
		}
	}
	boost::int32_t do_RepeatPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();

		const TokenStringTableItem &ci = tdata[pc0].str;
		std:: pair<size_t, size_t> underUpper(ci.range.first, ci.range.second);
		if (underUpper.second == 999) {
			underUpper.second = ~(size_t)0; //std:: numeric_limits<size_t>().max();
		}

		boost::int32_t pc = pc0 + 1;
		
		// an optimization
		{
			const TRACE_ITEM &item = tdata[pc].item;
			if (item.classification == TRACE_ITEM::Enter && item.node == NC_LiteralPattern) {
				boost::int32_t p = pc + 1;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_LiteralPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				boost::int32_t npos = substrEqualWithCharClassRepeated(source, pos0, p, underUpper);
				if (npos >= 0) {
					pMatchSeq->push_back(MATCH::makeCopy(&source, pos0, npos));
					return npos;
				}
				else {
					return -1;
				}
			}
		}

		size_t repeatCount = 0;
		boost::int32_t pos = pos0;
		while (repeatCount < underUpper.second) {
			const TRACE_ITEM &item = tdata[pc].item;
			assert(item.classification == TRACE_ITEM::Enter);
			extendNullMatchSeq(source, &pos);
			boost::int32_t npos = eval(pc, source, pos);
			assert(errorPc == -1);
			if (npos < 0) {
				break; // while
			}
			else if (npos == pos) {
				// when it matches a zero-length, exit this loop
				++repeatCount;
				break; // while
			}
			else {
				pos = npos;
				++repeatCount;
			}
		}
		if (! (underUpper.first <= repeatCount && repeatCount <= underUpper.second)) {
			pMatchSeq->resize(size0);
			return -1;
		}
		sequence_optimization(size0);
		return pos;
	}
	static boost::int32_t substrEqual(const text::TokenSequence &str, boost::int32_t pos0, const std:: vector<MYWCHAR_T> &token)
	{
		boost::int32_t pos = pos0;
		size_t i = 0; 
		while (true) {
			if (! (pos < str.size())) {
				return -1;
			}
			const text::Token *pToken = str.refAt(pos);
			assert(pToken != NULL);
			boost::optional<MYWCHAR_T> r;
			if (! (r = pToken->getRawCharCode()) || *r != token[i]) {
				return -1;
			}
			
			++i;
			++pos;

			if (! (i < token.size())) {
				break; // while
			}
			
			skip_null(str, &pos);
		}

		return pos;
	}
	boost::int32_t substrEqualWithCharClass(const text::TokenSequence &source, boost::int32_t pos0, boost::int32_t pc) const
	{
		const TokenStringTableItem &ci = tdata[pc].str;
		if (ci.isCharClass) {
			if (pos0 < source.size()) {
				assert(source.refAt(pos0) != NULL);
				boost::optional<MYWCHAR_T> r = source.refAt(pos0)->getRawCharCode();
				MYWCHAR_T code;
				if (r && ci.range.first <= (code = *r) && code <= ci.range.second) {
					return pos0 + 1;
				}
			}
			return -1;
		}
		else {
			boost::int32_t npos = substrEqual(source, pos0, ci.str);
			return npos;
		}
	}
	boost::int32_t substrEqualWithCharClassRepeated(const text::TokenSequence &source, boost::int32_t pos0, boost::int32_t pc, const std:: pair<size_t, size_t> &underUpper) const
	{
		const TokenStringTableItem &ci = tdata[pc].str;
		size_t repeatCount = 0;
		boost::int32_t pos = pos0;
		boost::int32_t posLastNonNull = pos;
		if (ci.isCharClass) {
			while (repeatCount < underUpper.second && pos < source.size()) {
				assert(source.refAt(pos) != NULL);
				boost::optional<MYWCHAR_T> r = source.refAt(pos)->getRawCharCode();
				MYWCHAR_T code;
				if (! (r && ci.range.first <= (code = *r) && code <= ci.range.second)) {
					break; // while
				}
				++pos;
				++repeatCount;
				posLastNonNull = pos;
				skip_null(source, &pos);
			}
		}
		else {
			while (repeatCount < underUpper.second && pos < source.size()) {
				boost::int32_t npos = substrEqual(source, pos, ci.str);
				if (npos < 0) {
					break; // while
				}
				pos = npos;
				++repeatCount;
				posLastNonNull = pos;
				skip_null(source, &pos);
			}
		}
		if (underUpper.first <= repeatCount && repeatCount <= underUpper.second) {
			return posLastNonNull;
		}
		return -1;
	}
	boost::int32_t do_XcepPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		if (! (pos0 < source.size())) {
			return pos0;
		}

		boost::int32_t pc = pc0 + 1;

		while (true) {
			const TRACE_ITEM &item = tdata[pc].item;
			if (item.classification != TRACE_ITEM::Enter) {
				break;
			}
			if (item.node == NC_LiteralPattern) {
				boost::int32_t p = pc + 1;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_LiteralPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				boost::int32_t npos = substrEqualWithCharClass(source, pos0, p);
				if (npos >= 0) {
					return -1;
				}
			}
			else if (item.node == NC_GeneratedTokenPattern) {
				boost::int32_t p = pc + 1;
#if ! defined NDEBUG
				const TRACE_ITEM &citem = tdata[p].item;
				assert(citem.classification == TRACE_ITEM::Accept);
				assert(citem.node == NC_GeneratedTokenPattern);
				assert(citem.ref.classification != TOKEN::NUL);
#endif
				boost::int32_t code = tdata[p].label;
				assert(code != -1);
				if (code == cNULL) {
					NULL;
				}
				else if (code == cANY) {
					return -1;
				}
				else if (code == cRAW) {
					assert(source.refAt(pos0) != NULL);
					if (source.refAt(pos0)->isRawChar()) {
						return -1;
					}
				}
				else {
					assert(source.refAt(pos0) != NULL);
					boost::optional<boost::int32_t> r;
					if ((r = source.refAt(pos0)->getGeneratedCode()) && *r == code) {
						return -1;
					}
				}
			}
			else {
				assert(false);
				return -1;
			}
			pc += 2;
		}
		return pos0;
	}
	boost::int32_t do_PreqPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		boost::int32_t pos = do_XcepPattern(pc0, source, pos0);
		return pos == -1 ? pos0 : -1;
	}
	boost::int32_t do_LiteralPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		boost::int32_t p = pc0 + 1;
#if ! defined NDEBUG
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_LiteralPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t npos = substrEqualWithCharClass(source, pos0, p);
		if (npos >= 0) {
			pMatchSeq->push_back(MATCH::makeCopy(&source, pos0, npos));
			return npos;
		}
		else {
			return -1;
		}
	}
	boost::int32_t do_GeneratedTokenPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		boost::int32_t p = pc0 + 1;
#if ! defined NDEBUG
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_GeneratedTokenPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t code = tdata[p].label;
		assert(code != -1);
		if (! (pos0 < source.size())) {
			return -1;
		}
		if (code == cANY) {
			pMatchSeq->push_back(MATCH::makeCopy1(&source, pos0));
			return pos0 + 1;
		}
		else if (code == cRAW) {
			assert(source.refAt(pos0) != NULL);
			if (source.refAt(pos0)->isRawChar()) {
				pMatchSeq->push_back(MATCH::makeCopy1(&source, pos0));
				return pos0 + 1;
			}
		}
		else {
			assert(source.refAt(pos0) != NULL);
			boost::optional<boost::int32_t> r;
			if ((r = source.refAt(pos0)->getGeneratedCode()) && *r == code) {
				pMatchSeq->push_back(MATCH::makeCopy1(&source, pos0));
				return pos0 + 1;
			}
		}
		return -1;
	}
	boost::int32_t do_RecursePattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
		size_t size0 = pMatchSeq->size();

		boost::int32_t recPc = recursePc.back();
		boost::int32_t pos = do_Pattern(recPc, source, pos0);
		if (pos < 0) {
			pMatchSeq->resize(size0);
		}
		return pos;
	}
	boost::int32_t do_InsertPattern(boost::int32_t pc0, const text::TokenSequence &source, boost::int32_t pos0)
	{
		std:: vector<MATCH> * const pMatchSeq = &matchSeq;
#if ! defined NDEBUG
		boost::int32_t p = td.findPair(pc0);
		const TRACE_ITEM &citem = tdata[p].item;
		assert(citem.classification == TRACE_ITEM::Accept);
		assert(citem.node == NC_InsertPattern);
		assert(citem.ref.classification != TOKEN::NUL);
#endif
		boost::int32_t labelCode = tdata[pc0].label;

		pMatchSeq->push_back(MATCH::makePackBegin(labelCode));
		pMatchSeq->push_back(MATCH::makePackEnd());

		return pos0;
	}
};

#endif // INTERPRETER_H
