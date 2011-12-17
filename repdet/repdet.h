#if ! defined REPDET_H
#define REPDET_H

#include <cassert>
#include <vector>
#include <map>
#include "../common/hash_map_includer.h"
#include <algorithm>

#include <boost/cstdint.hpp>

namespace repdet {

struct Repetition {
public:
	std:: pair<size_t/* begin */, size_t/* end */> beginEnd;
	size_t unit;
	/*
	beginEnd shows the begining index and the ending index of the repetition.
	unit shows the length of the repeating sub-sequence.
	
	For example, from "x123123y", we can find a Repetition(1, 7, 3), 
	that is, beginEnd.first = 1, beginEnd.second = 7, and unit = 3.
	
	The length of the repetition can be calcluated by (beginEnd.secondo - beginEnd.first).
	Also, the position of the first appearance of the repeating sub-sequence  
	starts at beginEnd.first and ends at (beginEnd.first + unit).

	In the above example, the length of the repetition is 7 - 1 = 6,
	and the position of the first appearance of the repeating sub-sequence
	is from 1 to 4 (= 1 + 3).
	*/
public:
	Repetition(size_t begin_, size_t end_, size_t unit_)
		: beginEnd(begin_, end_), unit(unit_)
	{
		assert(unit == 0 || (beginEnd.second - beginEnd.first) % unit == 0);
	}
	Repetition(const Repetition &right)
		: beginEnd(right.beginEnd), unit(right.unit)
	{
	}
	Repetition()
		: beginEnd(0, 0), unit(0)
	{
	}
};

struct reppos {
public:
	boost::uint64_t value;
public:
	reppos(const reppos &right)
		: value(right.value)
	{
	}
	reppos()
		: value(0)
	{
	}
	reppos(boost::uint32_t begin, boost::uint32_t end)
		: value((((boost::uint64_t)begin) << 32) | end)
	{
	}
public:
	inline boost::uint32_t begin() const
	{
		return (boost::uint32_t)(value >> 32);
	}
	inline boost::uint32_t end() const
	{
		return (boost::uint32_t)value;
	}
	inline bool operator==(const reppos &right) const
	{
		return value == right.value;
	}
	inline bool operator<(const reppos &right) const
	{
		return value < right.value;
	}
	inline bool operator!=(const reppos &right) const
	{
		return ! operator==(right);
	}
};

#if ! defined NO_HASH_THINGS

#if defined _MSC_VER
class reppos_hash_compare : public HASH_COMPARE<reppos>
#else
class reppos_hash_compare
#endif
{
public:
	inline size_t operator()(const reppos &key) const
	{
		return key.begin() | key.end();
	}
	inline bool operator()(const reppos &key1, const reppos &key2) const
	{
		return key1 < key2;
	}
};


typedef HASH_MAP<reppos, Repetition, reppos_hash_compare> MapRepposRepitition;

#else

typedef std::map<reppos, Repetition> MapRepposRepitition;

#endif

template<typename Elem>
class RepetitionDetector {
private:
	static bool match(const typename std:: vector<Elem> &data, size_t pos1, size_t pos2, size_t length)
	{
		if (pos1 + length > data.size() || pos2 + length > data.size()) {
			return false;
		}

		for (size_t i = 0; i < length; ++i) {
			if (data[pos1 + i] != data[pos2 + i]) {
				return false;
			}
		}
		return true;
	}
#if 0
	static void find_repetitions_brute(std:: map<std:: pair<size_t/* begin */, size_t/* end */>, Repetition> *pReps, 
			const typename std:: vector<Elem> &data, size_t begin, size_t end, size_t upperLimit)
	{
		assert(0 <= begin && begin <= end && end <= data.size());
		if (upperLimit == 0) {
			upperLimit = data.size();
		}

		(*pReps).clear();
		for (size_t pos = begin; pos < end; ++pos) {
			for (size_t unit = 1; unit < upperLimit && unit < data.size() / 3 && unit < data.size() - pos; ++unit) {
				size_t p = pos;

				while (p + unit * 2 <= end && match(data, p, p + unit, unit)) {
					p += unit;
				}
				if (p - pos > 0) {
					Repetition rep(pos, p + unit, unit);
					assert(match(data, pos, pos + unit, p - pos));
					std:: map<std:: pair<size_t/* begin */, size_t/* end */>, Repetition>::const_iterator i = (*pReps).find(rep.beginEnd);
					if (i == (*pReps).end()) {
						(*pReps)[rep.beginEnd] = rep;
					}
					else {
						assert(i->second.unit < rep.unit);
					}
				}
			}
		}
	}
#endif
	static void find_repetitions_skipvec(MapRepposRepitition *pReps, 
			const typename std:: vector<Elem> &data, size_t begin, size_t end, size_t upperLimit)
	{
		assert(0 <= begin && begin <= end && end <= data.size());
		if (upperLimit == 0) {
			upperLimit = data.size();
		}

		(*pReps).clear();

		for (size_t pos = begin; pos < end; ++pos) {
			size_t unit = 1;
			size_t p = pos;

			while (p + unit * 2 <= end && match(data, p, p + unit, unit)) {
				p += unit;
			}
			if (p - pos > 0) {
				Repetition rep(pos, p + unit, unit);
				assert(match(data, pos, pos + unit, p - pos));
				reppos key(rep.beginEnd.first, rep.beginEnd.second);
				MapRepposRepitition::const_iterator i = (*pReps).find(key);
				if (i == (*pReps).end()) {
					(*pReps)[key] = rep;
				}
				else {
					assert(i->second.unit < rep.unit);
				}
			}
		}

		// build skip vector
		std:: vector<size_t/* pos */> skipVec;
		skipVec.resize(data.size());
		typename std:: map<std:: pair<Elem, Elem>, size_t/* pos */> lastAppear;
		skipVec[data.size() - 1] = 0; // mark of not found
		for (size_t i = end - 1; i-- > begin; ) {
			std:: pair<Elem, Elem> c = std:: pair<Elem, Elem>(data[i], data[i + 1]);
			typename std:: map<std:: pair<Elem, Elem>, size_t>::iterator ci = lastAppear.find(c);
			if (ci == lastAppear.end()) {
				skipVec[i] = 0; // mark of not found
				lastAppear[c] = i;
			}
			else {
				skipVec[i] = ci->second;
				ci->second = i;
			}
		}

		for (size_t pos = begin; pos < end; ++pos) {
			size_t nextPos = skipVec[pos];
			while (nextPos != 0) {
				size_t unit = nextPos - pos;
				if (! (unit < upperLimit && unit < end / 3 && unit < end - pos)) {
					break; // while
				}
				if (unit > 1) {
					if (pos + unit * 2 <= end && match(data, pos, pos + unit, unit)) {
						if (! (pos >= begin + unit /* i.e., pos - unit >= begin */ && match(data, pos - unit, pos, unit))) {
							size_t p = pos + unit;
							while (p + unit * 2 <= end && match(data, p, p + unit, unit)) {
								p += unit;
							}
							assert(p - pos > 0);
							Repetition rep(pos, p + unit, unit);
							assert(match(data, pos, pos + unit, p - pos));
							reppos key(rep.beginEnd.first, rep.beginEnd.second);
							MapRepposRepitition::const_iterator i = (*pReps).find(key);
							if (i == (*pReps).end()) {
								(*pReps)[key] = rep;
							}
							else {
								assert(unit == 1 || i->second.unit < rep.unit);
							}
						}
					}
				}
				nextPos = skipVec[nextPos];
			}
		}
	}
	static void find_repetitions_skipvec(std::vector</* begin index */ std::vector<Repetition> > *pReps, 
			const typename std:: vector<Elem> &data, size_t begin, size_t end, size_t upperLimit)
	{
		assert(0 <= begin && begin <= end && end <= data.size());
		if (upperLimit == 0) {
			upperLimit = data.size();
		}

		(*pReps).clear();
		(*pReps).reserve(end);

		for (size_t pos = begin; pos < end; ++pos) {
			size_t unit = 1;
			size_t p = pos;

			while (p + unit * 2 <= end && match(data, p, p + unit, unit)) {
				p += unit;
			}
			if (p - pos > 0) {
				Repetition rep(pos, p + unit, unit);
				assert(match(data, pos, pos + unit, p - pos));
				if (rep.beginEnd.first >= (*pReps).size()) {
					(*pReps).resize(rep.beginEnd.first + 1);
					(*pReps)[rep.beginEnd.first].push_back(rep);
				}
				else {
					std::vector<Repetition> &repsWithTheSameBegin = (*pReps)[rep.beginEnd.first];
					std::vector<Repetition>::iterator it = std::lower_bound(repsWithTheSameBegin.begin(), repsWithTheSameBegin.end(), rep, RepetitionComparatorByBeginEnd());
					if (it == repsWithTheSameBegin.end() || (*it).beginEnd != rep.beginEnd) {
						repsWithTheSameBegin.insert(it, rep);
					}
					else {
						assert(unit == 1 || (*it).unit < rep.unit);
					}
				}
			}
		}

		// build skip vector
		std:: vector<size_t/* pos */> skipVec;
		skipVec.resize(data.size());
		typename std:: map<std:: pair<Elem, Elem>, size_t/* pos */> lastAppear;
		skipVec[data.size() - 1] = 0; // mark of not found
		for (size_t i = end - 1; i-- > begin; ) {
			std:: pair<Elem, Elem> c = std:: pair<Elem, Elem>(data[i], data[i + 1]);
			typename std:: map<std:: pair<Elem, Elem>, size_t>::iterator ci = lastAppear.find(c);
			if (ci == lastAppear.end()) {
				skipVec[i] = 0; // mark of not found
				lastAppear[c] = i;
			}
			else {
				skipVec[i] = ci->second;
				ci->second = i;
			}
		}

		for (size_t pos = begin; pos < end; ++pos) {
			size_t nextPos = skipVec[pos];
			while (nextPos != 0) {
				size_t unit = nextPos - pos;
				if (! (unit < upperLimit && unit < end / 3 && unit < end - pos)) {
					break; // while
				}
				if (unit > 1) {
					size_t p = pos;
					
					while (p + unit * 2 <= end && match(data, p, p + unit, unit)) {
						p += unit;
					}
					if (p - pos > 0) {
						Repetition rep(pos, p + unit, unit);
						assert(match(data, pos, pos + unit, p - pos));
						if (rep.beginEnd.first >= (*pReps).size()) {
							(*pReps).resize(rep.beginEnd.first + 1);
							(*pReps)[rep.beginEnd.first].push_back(rep);
						}
						else {
							std::vector<Repetition> &repsWithTheSameBegin = (*pReps)[rep.beginEnd.first];
							std::vector<Repetition>::iterator it = std::lower_bound(repsWithTheSameBegin.begin(), repsWithTheSameBegin.end(), rep, RepetitionComparatorByBeginEnd());
							if (it == repsWithTheSameBegin.end() || (*it).beginEnd != rep.beginEnd) {
								repsWithTheSameBegin.insert(it, rep);
							}
							else {
								assert(unit == 1 || (*it).unit < rep.unit);
							}
						}
					}
				}
				nextPos = skipVec[nextPos];
			}
		}
	}
private:
	class RepetitionComparatorByBeginEnd
	{
	public:
		inline bool operator()(const Repetition &left, const Repetition &right) const
		{
			return left.beginEnd < right.beginEnd;
		}
	};
public:
	void findRepetitions(MapRepposRepitition *pReps, 
			const typename std:: vector<Elem> &data, size_t upperLimit /* special value 0 means +infinity */) const
	{
		find_repetitions_skipvec(pReps, data, 0, data.size(), upperLimit);
	}
	void findRepetitions(std:: vector<Repetition> *pReps, 
			const typename std:: vector<Elem> &data, size_t upperLimit /* special value 0 means +infinity */) const
	{
		MapRepposRepitition repm;
		find_repetitions_skipvec(&repm, data, 0, data.size(), upperLimit);
		(*pReps).clear();
		(*pReps).reserve(repm.size());
		for (MapRepposRepitition::const_iterator it = repm.begin(); it != repm.end(); ++it) {
			(*pReps).push_back(it->second);
		}
		std::sort((*pReps).begin(), (*pReps).end(), RepetitionComparatorByBeginEnd());
	}
	void findRepetitions(MapRepposRepitition *pReps, 
			const std:: vector<Elem> &data, size_t begin, size_t end, 
			size_t upperLimit /* special value 0 means +infinity */) const
	{
		find_repetitions_skipvec(pReps, data, begin, end, upperLimit);
	}
	void findRepetitions(std:: vector<Repetition> *pReps, 
			const std:: vector<Elem> &data, 
			size_t begin, size_t end, 
			size_t upperLimit /* special value 0 means +infinity */) const
	{
		MapRepposRepitition repm;
		find_repetitions_skipvec(&repm, data, begin, end, upperLimit);
		(*pReps).clear();
		(*pReps).reserve(repm.size());
		for (MapRepposRepitition::const_iterator it = repm.begin(); it != repm.end(); ++it) {
			(*pReps).push_back(it->second);
		}
		std::sort((*pReps).begin(), (*pReps).end(), RepetitionComparatorByBeginEnd());
	}
};

}; // namespace repdet

#endif // REPDET_H
