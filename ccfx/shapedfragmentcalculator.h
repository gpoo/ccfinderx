#if ! defined SHAPED_FRAGMENT_CALCULATOR_H
#define SHAPED_FRAGMENT_CALCULATOR_H

#include <vector>
#include <utility>
#include <cassert>
#include <algorithm>

#include <boost/optional.hpp>

#include "../common/hash_set_includer.h"

namespace shaper {

struct ShapedFragmentPosition {
public:
	int depth;
	size_t begin;
	size_t end;
public:
	ShapedFragmentPosition()
		: depth(0), begin(0),end(0)
	{
	}
	ShapedFragmentPosition(int depth_, size_t begin_, size_t end_)
		: depth(depth_), begin(begin_), end(end_)
	{
	}
	ShapedFragmentPosition(const ShapedFragmentPosition &right)
		: depth(right.depth), begin(right.begin), end(right.end)
	{
	}
public:
	void swap(ShapedFragmentPosition &right)
	{
		std:: swap(*this, right);
	}
};

enum FRAGMENT_TYPE { NONE_FRAGMENT, HAT_FRAGMENT, CAP_FRAGMENT };

template<typename ElemType>
class ShapedFragmentsCalculator {
private:
	std::vector<ElemType> parens;
	size_t parenCount;
	HASH_SET<ElemType> prefixes;
	HASH_SET<ElemType> suffixes;
	size_t minLength;

	const std:: vector<ElemType> *pSeq;
public:
	ShapedFragmentsCalculator()
		: parens(), parenCount(0), prefixes(), suffixes(), minLength(1), pSeq(NULL)
	{
	}
public:
	void setParens(const std:: vector<std:: pair<ElemType, ElemType> > &parens_)
	{
		parens.clear();
		parens.reserve(parens_.size() * 2);
		parenCount = parens_.size();
		for (size_t pi = 0; pi < parens_.size(); ++pi) {
			parens.push_back(parens_[pi].first);
		}
		for (size_t pi = 0; pi < parens_.size(); ++pi) {
			parens.push_back(parens_[pi].second);
		}
		assert(parens.size() == parenCount * 2);
	}
	void setPrefixes(const std::vector<ElemType> &prefixes_)
	{
		prefixes.clear(); // 2008/02/04
		prefixes.insert(prefixes_.begin(), prefixes_.end());
	}
	void setSuffixes(const std::vector<ElemType> &suffixes_)
	{
		suffixes.clear(); // 2008/02/04
		suffixes.insert(suffixes_.begin(), suffixes_.end());
	}
	void setMinlengh(size_t minLength_)
	{
		minLength = minLength_;
		if (minLength < 1) {
			minLength = 1;
		}
	}
	void calc(
			std:: vector<ShapedFragmentPosition> *pFragments,
			const std:: vector<ElemType> &seq, size_t begin, size_t end, FRAGMENT_TYPE hat_or_cap)
	{
		assert(end <= seq.size());

		pSeq = &seq;
		(*pFragments).clear();

		std:: pair<int, size_t> curPos(0, begin);
		while (curPos.second < end) {
			std:: vector<ShapedFragmentPosition> fragments;
			std:: pair<int, size_t> nextPos = calc_i(&fragments, curPos, end);
			(*pFragments).insert((*pFragments).end(), fragments.begin(), fragments.end());
			curPos = nextPos;
		}

		if (hat_or_cap == CAP_FRAGMENT) {
			std:: vector<ShapedFragmentPosition> fs;
			fs.reserve((*pFragments).size());
			for (size_t i = 0; i < (*pFragments).size(); ++i) {
				const ShapedFragmentPosition &pos = (*pFragments)[i];
				ShapedFragmentPosition p(pos);
				size_t &b = p.begin;
				for (b = pos.begin; b < pos.end && ! isOpenParen(seq[b]); ++b)
					NULL;
				if (b != pos.end) {
					size_t &e = p.end;
					for (e = pos.end; e >= b + minLength && ! isCloseParen(seq[e - 1]); --e)
						NULL;
					if (e >= b + minLength) {
						assert(isCloseParen(seq[e - 1]));
						fs.push_back(p);
					}
				}
			}
			(*pFragments).swap(fs);
		}
	}
	boost::optional<ShapedFragmentPosition> findAtLeastOne(
			const std:: vector<ElemType> &seq, size_t begin, size_t end, FRAGMENT_TYPE hat_or_cap)
	{
		assert(end <= seq.size());

		pSeq = &seq;

		std:: pair<int, size_t> curPos(0, begin);
		while (curPos.second < end) {
			std:: vector<ShapedFragmentPosition> fragments;
			std:: pair<int, size_t> nextPos = calc_i(&fragments, curPos, end);

			if (hat_or_cap == CAP_FRAGMENT) {
				for (size_t i = 0; i < fragments.size(); ++i) {
					const ShapedFragmentPosition &pos = fragments[i];
					ShapedFragmentPosition p(pos);
					size_t &b = p.begin;
					for (b = pos.begin; b < pos.end && ! isOpenParen(seq[b]); ++b)
						NULL;
					if (b != pos.end) {
						size_t &e = p.end;
						for (e = pos.end; e >= b + minLength && ! isCloseParen(seq[e - 1]); --e)
							NULL;
						if (e >= b + minLength) {
							assert(isCloseParen(seq[e - 1]));
							boost::optional<ShapedFragmentPosition> r = p;
							return r;
						}
					}
				}
			}
			else {
				if (! fragments.empty()) {
					boost::optional<ShapedFragmentPosition> r = fragments[0];
					return r;
				}
			}
			curPos = nextPos;
		}
		boost::optional<ShapedFragmentPosition> r;
		return r;
	}
private:
	void removePrefixAndSuffix(ShapedFragmentPosition *pfragment)
	{
		const std:: vector<ElemType> &seq = *pSeq;
		ShapedFragmentPosition &fragment = *pfragment;
		while (fragment.begin < fragment.end && suffixes.find(seq[fragment.begin]) != suffixes.end()) {
			++fragment.begin;
		}
		while (fragment.begin < fragment.end && prefixes.find(seq[fragment.end - 1]) != prefixes.end()) {
			--fragment.end;
		}
	}
	inline bool isOpenParen(const ElemType &token) const
	{
		return std::find(parens.begin(), parens.begin() + parenCount, token) != parens.begin() + parenCount;
	}
	inline bool isCloseParen(const ElemType &token) const
	{
		return std::find(parens.begin() + parenCount, parens.end(), token) != parens.end();
	}
	inline bool isNotParen(const ElemType &token) const 
	{
		return std::find(parens.begin(), parens.end(), token) == parens.end();
	}
	std:: pair<int, size_t> calc_i(std:: vector<ShapedFragmentPosition> *pFragments,
		const std:: pair<int, size_t> &beginPos, size_t end)
	{
		int depth0 = beginPos.first;
		std:: pair<int, size_t> pos = beginPos;
		std:: pair<int, size_t> flatEndPos = pos;
		
		assert(end <= (*pSeq).size());
		while (pos.second < end) {
			ElemType token = (*pSeq)[pos.second];
			if (isOpenParen(token)) {
				++pos.first;
				++pos.second;
			}
			else if (isCloseParen(token)) {
				--pos.first;
				++pos.second;
				if (pos.first < depth0) {
					ShapedFragmentPosition fragment(beginPos.first, beginPos.second, pos.second - 1);
					removePrefixAndSuffix(&fragment);
					if (fragment.end - fragment.begin >= minLength) {
						(*pFragments).push_back(fragment);
					}
					while (pos.second < end && isCloseParen((*pSeq)[pos.second])) {
						--pos.first;
						++pos.second;
					}
					return pos;
				}
			}
			else {
				++pos.second;
			}
			if (pos.first == depth0) {
				flatEndPos = pos;
			}
		}
		if (pos.first == depth0) {
			assert(pos.second == end);
			ShapedFragmentPosition fragment(beginPos.first, beginPos.second, pos.second);
			removePrefixAndSuffix(&fragment);
			if (fragment.end - fragment.begin >= minLength) {
				(*pFragments).push_back(fragment);
			}
			return pos;
		}

		{
			ShapedFragmentPosition fragment(beginPos.first, beginPos.second, flatEndPos.second);
			removePrefixAndSuffix(&fragment);
			if (fragment.end - fragment.begin >= minLength) {
				(*pFragments).push_back(fragment);
			}
		}
		
		pos = flatEndPos;
		while (pos.second < end && isOpenParen((*pSeq)[pos.second])) {
			++pos.first;
			++pos.second;
		}
		return pos;
	}
};

}; // namespace

#endif // SHAPED_FRAGMENT_CALCULATOR_H
