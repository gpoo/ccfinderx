#if ! defined CLONEDETECTOR_H
#define CLONEDETECTOR_H

#include <cassert>
#include <vector>
#include <map>
#include "../common/hash_map_includer.h"
#include <algorithm>
#include <limits>
#include <iterator> 

#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>

#include "../threadqueue/threadqueue.h"

#if defined _MSC_VER
#undef max
#undef min
#endif

template<typename ElemType, typename HashValueType>
class CloneDetector {
private:
	class SubSequence {
	private:
		size_t begin;
		size_t end;
	public:
		SubSequence(size_t beginPos_, size_t endPos_)
			: begin(beginPos_), end(endPos_)
		{
			assert(beginPos_ <= endPos_);
		}
		SubSequence()
			: begin(), end()
		{
		}
		SubSequence(const SubSequence &right)
			: begin(right.begin), end(right.end)
		{
		}
	public:
		//bool operator==(const SubSequence &right) const
		//{
		//	if (end - begin != right.end - right.begin) {
		//		return false;
		//	}
		//	const std:: vector<ElemType> &seq = *pSeq;
		//	size_t li = begin;
		//	size_t ri = right.begin;
		//	while (li != end) {
		//		ElemType lt = to_compared(seq, li, begin);
		//		ElemType rt = to_compared(*right.pSeq, ri, right.begin);
		//		if (lt != rt) {
		//			return false;
		//		}
		//		++li;
		//		++ri;
		//	}
		//	return true;
		//}
		//bool operator<(const SubSequence &right) const
		//{
		//	const std:: vector<ElemType> &seq = *pSeq;
		//	size_t li = begin;
		//	size_t ri = right.begin;
		//	while (li < end && ri < right.end) {
		//		ElemType lt = to_compared(seq, li, begin);
		//		ElemType rt = to_compared(*right.pSeq, ri, right.begin);
		//		if (lt != rt) {
		//			break; // while
		//		}
		//		++li;
		//		++ri;
		//	}
		//	
		//	if (li < end) {
		//		if (ri < right.end) {
		//			ElemType lt = to_compared(seq, li, begin);
		//			ElemType rt = to_compared(*right.pSeq, ri, right.begin);
		//			if (lt < rt) {
		//				return true;
		//			}
		//			else {
		//				return false;
		//			}
		//		}
		//		else {
		//			assert(ri == right.end);
		//			return false;
		//		}
		//	}
		//	else {
		//		assert(li == end);
		//		if (ri < right.end) {
		//			return true;
		//		}
		//		else {
		//			assert(ri == right.end);
		//			return false;
		//		}
		//	}
		//}
		//const ElemType &operator[](size_t index) const
		//{
		//	return *(li + index);
		//}
		void swap(SubSequence &right)
		{
			std:: swap(this->begin, right.begin);
			std:: swap(this->end, right.end);
		}
		inline size_t size() const
		{
			return end - begin;
		}
		inline size_t getBegin() const
		{
			return this->begin;
		}
		inline size_t getEnd() const
		{
			return this->end;
		}
	public:
		class SequencePrevComparator {
		private:
			size_t unitLength;
			const typename std:: vector<ElemType> *pSeq;
		public:
			SequencePrevComparator(size_t unitLength_, const std:: vector<ElemType> *pSeq_)
				: unitLength(unitLength_), pSeq(pSeq_)
			{
			}
			SequencePrevComparator()
				: unitLength(0), pSeq(NULL)
			{
			}
			SequencePrevComparator(const SequencePrevComparator &right)
				: unitLength(right.unitLength), pSeq(right.pSeq)
			{
			}
			bool operator()(size_t posLeft, size_t posRight) const
			{
				assert(posLeft + unitLength <= (*pSeq).size());
				assert(posRight + unitLength <= (*pSeq).size());
				size_t i;
				for (i = 0; i < unitLength; ++i) {
					const ElemType &li = to_compared(*pSeq, posLeft + i, posLeft);
					const ElemType &ri = to_compared(*pSeq, posRight + i, posRight);
					assert(li != 0);
					assert(ri != 0);
					if (li != ri) {
						break; // for i
					}
				}
				if (i != unitLength) {
					const ElemType &li = to_compared(*pSeq, posLeft + i, posLeft);
					const ElemType &ri = to_compared(*pSeq, posRight + i, posRight);
					return li < ri;
				}
				else {
					const ElemType &lp = to_reversereference_compared(*pSeq, posLeft - 1, posLeft, posLeft + unitLength);
					const ElemType &rp = to_reversereference_compared(*pSeq, posRight - 1, posRight, posRight + unitLength);
					return lp < rp;
				}
			}
		};
		class ExtensionPrevComparator {
		private:
			size_t baseLength;
			const typename std:: vector<ElemType> *pSeq;
		public:
			ExtensionPrevComparator(size_t baseLength_, const std:: vector<ElemType> *pSeq_)
				: baseLength(baseLength_), pSeq(pSeq_)
			{
			}
			ExtensionPrevComparator()
				: baseLength(0), pSeq(NULL)
			{
			}
			ExtensionPrevComparator(const ExtensionPrevComparator &right)
				: baseLength(right.baseLength), pSeq(right.pSeq)
			{
			}
			bool operator()(size_t posLeft, size_t posRight) const
			{
				assert(posLeft + baseLength < (*pSeq).size());
				assert(posRight + baseLength < (*pSeq).size());
				const ElemType &li = to_compared(*pSeq, posLeft + baseLength, posLeft);
				const ElemType &ri = to_compared(*pSeq, posRight + baseLength, posRight);
				if (li == ri) {
					const ElemType &lp = to_reversereference_compared(*pSeq, posLeft - 1, posLeft, posLeft + baseLength);
					const ElemType &rp = to_reversereference_compared(*pSeq, posRight - 1, posRight, posRight + baseLength);
					return lp < rp;
				}
				else {
					return li < ri;
				}
			}
		};
		class PrevExtensionComparator {
		private:
			size_t baseLength;
			const typename std:: vector<ElemType> *pSeq;
		public:
			PrevExtensionComparator(size_t baseLength_, const std:: vector<ElemType> *pSeq_)
				: baseLength(baseLength_), pSeq(pSeq_)
			{
			}
			PrevExtensionComparator()
				: baseLength(0), pSeq(NULL)
			{
			}
			PrevExtensionComparator(const ExtensionPrevComparator &right)
				: baseLength(right.baseLength), pSeq(right.pSeq)
			{
			}
			bool operator()(size_t posLeft, size_t posRight) const
			{
				const ElemType &lp = to_reversereference_compared(*pSeq, posLeft - 1, posLeft, posLeft + baseLength);
				const ElemType &rp = to_reversereference_compared(*pSeq, posRight - 1, posRight, posRight + baseLength);
				if (lp == rp) {
					assert(posLeft + baseLength < (*pSeq).size());
					assert(posRight + baseLength < (*pSeq).size());
					const ElemType &li = to_compared(*pSeq, posLeft + baseLength, posLeft);
					const ElemType &ri = to_compared(*pSeq, posRight + baseLength, posRight);
					return li < ri;
				}
				else {
					return lp < rp;
				}
			}
		};
	};
	static bool subsequenceEqual(const typename std:: vector<ElemType> *pSeq, const SubSequence &left, const SubSequence &right)
	{
		if (left.size() != right.size()) {
			return false;
		}
		size_t li = left.getBegin();
		size_t ri = right.getBegin();
		while (li != left.getEnd()) {
			ElemType lt = to_compared(*pSeq, li, left.getBegin());
			ElemType rt = to_compared(*pSeq, ri, right.getBegin());
			if (lt != rt) {
				return false;
			}
			++li;
			++ri;
		}
		return true;
	}
public:
	class SequenceHashFunction {
	public:
		virtual ~SequenceHashFunction()
		{
		}
		virtual HashValueType operator()(const typename std:: vector<ElemType> &seq, size_t begin, size_t end) = 0;
	};
public:
	struct CloneSetItem {
	public:
		ElemType prev;
		ElemType extension;
		std::vector<size_t/* pos */> poss;
	public:
		CloneSetItem(const CloneSetItem &right)
			: prev(right.prev), extension(right.extension), poss(right.poss)
		{
		}
		CloneSetItem()
			: prev(0), extension(0), poss()
		{
		}
	};
	class CloneSetListener {
	private:
		const std:: vector<ElemType> *pSeq;
		size_t unitLength;
	public:
		virtual ~CloneSetListener()
		{
		}
		CloneSetListener()
			: pSeq(NULL), unitLength(0)
		{
		}
		CloneSetListener(const CloneSetListener &right)
			: pSeq(right.pSeq), unitLength(right.unitLength)
		{
		}
	public:
		virtual void attachSeq(const std:: vector<ElemType> *pSeq_)
		{
			pSeq = pSeq_;
			const std:: vector<ElemType> &seq = *pSeq;
			if (! seq.empty()) {
				assert(seq[0] == 0);
				assert(seq.back() == 0);
			}
		}
		virtual void setUnitLength(size_t unitLength_)
		{
			unitLength = unitLength_;
		}
	public:
		virtual bool rangeCheck(const std:: vector<CloneSetItem> &cloneSet)
		{
			return true;
		}
		virtual bool codeCheck(size_t pos, size_t length)
		{
			return true;
		}
		virtual void found(const std:: vector<CloneSetItem> &cloneSet, size_t baseLength, boost::uint64_t cloneSetReferenceNumber)
		{
		}
	protected:
		const std:: vector<ElemType> &refSeq() const
		{
			return *pSeq;
		}
		size_t getUnitLength() const
		{
			return unitLength;
		}
	};
	class ClonePairListener {
	private:
		const typename std:: vector<ElemType> *pSeq;
		size_t unitLength;
	public:
		virtual ~ClonePairListener()
		{
		}
		ClonePairListener()
			: pSeq(NULL), unitLength(0)
		{
		}
		ClonePairListener(const CloneSetListener &right)
			: pSeq(right.pSeq), unitLength(right.unitLength)
		{
		}
	public:
		virtual void attachSeq(const std:: vector<ElemType> *pSeq_)
		{
			pSeq = pSeq_;
		}
		virtual void setUnitLength(size_t unitLength_)
		{
			unitLength = unitLength_;
		}
	public:
		virtual bool codeCheck(size_t pos, size_t length)
		{
			return true;
		}
		virtual bool rangeCheck(const std:: vector<CloneSetItem> &cloneSet)
		{
			return true;
		}
		virtual void found(size_t pos1, size_t pos2, size_t baseLength, boost::uint64_t cloneSetReferenceNumber)
		{
			assert(pos1 < pos2);
		}
	protected:
		const typename std:: vector<ElemType> &refSeq() const
		{
			return *pSeq;
		}
		size_t getUnitLength() const
		{
			return unitLength;
		}
	};
	class ClonePairListenerWithScope : public ClonePairListener {
	private:
		size_t barrior;
		enum mode_t { mode_all, mode_left_and_cross, mode_cross } mode;
	public:
		virtual ~ClonePairListenerWithScope()
		{
		}
		ClonePairListenerWithScope()
			: ClonePairListener(), barrior(0), mode(mode_all)
		{
		}
		ClonePairListenerWithScope(const CloneSetListener &right)
			: ClonePairListener(right), barrior(right.barrior), mode(right.mode)
		{
		}
	public:
		void setAllMode()
		{
			mode = mode_all;
		}
		void setCrossMode(size_t barrior_)
		{
			mode = mode_cross;
			barrior = barrior_;
		}
		void setLeftAndCrossMode(size_t barrior_)
		{
			mode = mode_left_and_cross;
			barrior = barrior_;
		}
	public:
		virtual bool rangeCheck(const std:: vector<CloneSetItem> &cloneSet)
		{
			switch (mode) {
			case mode_all:
				{
					return true;
				}
				break;
			case mode_left_and_cross:
				{
					for (size_t i = 0; i < cloneSet.size(); ++i) {
						const std::vector<size_t> &poss = cloneSet[i].poss;
						for (std:: vector<size_t/* pos */>::const_iterator j = poss.begin(); j != poss.end(); ++j) {
							if (*j < barrior) {
								return true;
							}
						}
					}
				}
				break;
			case mode_cross:
				{
					bool leftFound = false;
					bool rightFound = false;
					for (size_t i = 0; i < cloneSet.size(); ++i) {
						const std::vector<size_t> &poss = cloneSet[i].poss;
						for (std:: vector<size_t/* pos */>::const_iterator j = poss.begin(); j != poss.end(); ++j) {
							if (*j < barrior) {
								leftFound = true;
							}
							else {
								rightFound = true;
							}
							if (leftFound && rightFound) {
								return true;
							}
						}
					}
				}
				break;
			default:
				assert(false);
				break;
			}
			return false;
		}
		virtual void found(size_t posA, size_t posB, size_t baseLength, boost::uint64_t cloneSetReferenceNumber)
		{
			assert(posA < posB);
			switch (mode) {
			case mode_all:
				break;
			case mode_left_and_cross:
				if (posA >= barrior && posB >= barrior) {
					return;
				}
				break;
			case mode_cross:
				if (posA >= barrior && posB >= barrior || posA < barrior && posB < barrior) {
					return;
				}
				break;
			default:
				assert(false);
				break;
			}
			found_scoped(posA, posB, baseLength, cloneSetReferenceNumber);
		}
	protected:
		virtual void found_scoped(size_t pos1, size_t pos2, size_t baseLength, boost::uint64_t cloneSetReferenceNumber)
		{
			assert(pos1 < pos2);
		}
	};
private:
	class ClonePairListenerAdapter : public CloneSetListener {
	private:
		ClonePairListener *pListener;
	public:
		ClonePairListenerAdapter(ClonePairListener *pRight)
			: pListener(pRight)
		{
		}
	public:
		virtual void attachSeq(const typename std:: vector<ElemType> *pSeq_)
		{
			CloneSetListener::attachSeq(pSeq_);
			(*pListener).attachSeq(pSeq_);
		}
		virtual void setUnitLength(size_t unitLength_)
		{
			CloneSetListener::setUnitLength(unitLength_);
			(*pListener).setUnitLength(unitLength_);
		}
	public:
		virtual bool codeCheck(size_t pos, size_t length)
		{
			return (*pListener).codeCheck(pos, length);
		}
		virtual bool rangeCheck(const std:: vector<CloneSetItem> &cloneSet)
		{
			return (*pListener).rangeCheck(cloneSet);
		}
		virtual void found(const std:: vector<CloneSetItem> &cloneSet, size_t baseLength, 
				boost::uint64_t cloneSetReferenceNumber)
		{
			//const typename std:: vector<ElemType> &seq = refSeq();
			size_t unitLength = getUnitLength();

			for (size_t csi = 0; csi < cloneSet.size(); ++csi) {
				const CloneSetItem &cs = cloneSet[csi];
				for (size_t csj = csi; csj < cloneSet.size(); ++csj) { // 2008/02/13
					const CloneSetItem &right = cloneSet[csj];
					if ((cs.prev == 0 || cs.prev != right.prev) && (cs.extension == 0 || cs.extension != right.extension)) {
						const std::vector<size_t> &poss = cs.poss;
						for (std:: vector<size_t/* pos */>::const_iterator a = poss.begin(); a != poss.end(); ++a) {
							const std::vector<size_t> &possRight = right.poss;
							for (std:: vector<size_t/* pos */>::const_iterator b = (&cs == &right) ? a + 1 : possRight.begin(); b != possRight.end(); ++b) {
								size_t posA = *a;
								size_t posB = *b;
								assert(posA != posB);
								if (posA < posB) {
									(*pListener).found(posA, posB, baseLength, cloneSetReferenceNumber);
								}
								else {
									(*pListener).found(posB, posA, baseLength, cloneSetReferenceNumber);
								}
							}
						}
					}
				}
			}
		}
	};
private:
	const typename std:: vector<ElemType> *pSeq;
	size_t bottomUnitLength;
	size_t multiply;
	std:: vector<HashValueType> hashSeq;
	//bool optionVerbose;
	boost::uint64_t cloneSetReferenceNumber;
	size_t numThreads;
public:
	CloneDetector()
		: pSeq(NULL), bottomUnitLength(0), multiply(1), hashSeq()/*, optionVerbose(false)*/, cloneSetReferenceNumber(0), numThreads(1)
	{
	}
	CloneDetector(const CloneDetector &right)
		: pSeq(right.pSeq), bottomUnitLength(right.bottomUnitLength), multiply(right.multiply), hashSeq(right.hashSeq)/*, optionVerbose(right.optionVerbose)*/, numThreads(1)
	{
	}
private:
	CloneDetector(size_t dummy) // dummy to ensure methods
	{
		std::vector<ElemType> seqDummy;
		size_t pos = 0;
		size_t begin = 0;
		size_t end = 0;
		ElemType t1 = to_compared(&seqDummy, pos, begin);
		ElemType t2 = to_reversereference_compared(&seqDummy, pos, begin, end);
		assert(false);
	}
public:
	void setThreads(size_t numThreads_)
	{
		numThreads = numThreads_;
	}
	void attachSequence(const std:: vector<ElemType> *pSeq_)
	{
		assert(pSeq_ != NULL);
		pSeq = pSeq_;
	}
	const typename std:: vector<ElemType> &refSeq() const
	{
		return *pSeq;
	}
	void detachSequence()
	{
		pSeq = NULL;
	}
	void setBottomUnitLength(size_t bottomUnitLength_)
	{
		bottomUnitLength = bottomUnitLength_;
	}
	void setMultiply(size_t multiply_)
	{
		multiply = multiply_;
	}
	size_t getUnitLength() const
	{
		return bottomUnitLength * multiply;
	}
//	void setOptionVerbose(bool ov)
//	{
//		optionVerbose = ov;
//	}
//	bool getOptionVerbose() const
//	{
//		return optionVerbose;
//	}
	void clearCloneSetReferenceNumber()
	{
		cloneSetReferenceNumber = 0;
	}
	void findClonePair(ClonePairListener *pListener, SequenceHashFunction &hashFunc)
	{
		ClonePairListenerAdapter a(pListener);
		findCloneSet(&a, hashFunc);
	}
public:
	void print_seq(size_t beginPos, size_t len)
	{
		const std:: vector<ElemType> &seq = *pSeq;
		size_t endPos = beginPos + len;

		size_t count = 0;
		for (size_t i = beginPos; i < endPos; ++i) {
			if (count > 0) {
				std::cout << " ";
			}
			std::cout << (int)(seq[i]);
			++count;
			if (count == 10) {
				std::cout << std::endl;
				count = 0;
			}
		}
	}
private:
	struct CloneSetData {
		std::vector<CloneSetItem> cloneSet;
		size_t baseLength;
	};
	void send_clone_set_data_to_listener(ThreadQueue<std::vector<std::vector<CloneSetData> > *> *pQue, CloneSetListener *pListener) {
		std::vector<std::vector<CloneSetData> > *pFoundCloneSetsForThreads;
		while ((pFoundCloneSetsForThreads = (*pQue).pop()) != NULL) {
			std::vector<std::vector<CloneSetData> > &foundCloneSetsForThreads = *pFoundCloneSetsForThreads;
			for (size_t cii = 0; cii < foundCloneSetsForThreads.size(); ++cii) {
				std::vector<CloneSetData> &foundCloneSets = foundCloneSetsForThreads[cii];
				for (size_t csi = 0; csi < foundCloneSets.size(); ++csi) {
					++cloneSetReferenceNumber;
					const CloneSetData &cloneSetData = foundCloneSets[csi];
					(*pListener).found(cloneSetData.cloneSet, cloneSetData.baseLength, cloneSetReferenceNumber);
				}
			}
			delete pFoundCloneSetsForThreads;
		}
	}
public:
	void findCloneSet(CloneSetListener *pListener, SequenceHashFunction &hashFunc)
	{
		const std:: vector<ElemType> &seq = *pSeq;
		const size_t unitLength = getUnitLength();

		//if (optionVerbose) {
		//	std:: cerr << "> finding identical substrings" << std:: endl;
		//}

		calc_hash_seq(hashFunc);

		//std::vector<HashValueType> hashSeqCopy = hashSeq;
		//calc_hash_seq_prev_version(hashFunc);
		//assert(hashSeq.size() == hashSeqCopy.size());
		//for (size_t i = 0; i < hashSeq.size(); ++i) {
		//	assert(hashSeq[i] == hashSeqCopy[i]);
		//}

		//for (size_t i = 0; i < hashSeq.size(); ++i) {
		//	std:: cout << i << ": " << hashSeq[i] << " " << std:: endl;
		//}

		(*pListener).attachSeq(&seq);
		(*pListener).setUnitLength(unitLength);
		
		if (seq.size() < unitLength) {
			return;
		}

		std::vector<std:: vector<size_t/* pos */> > cloneFragments;
		{
			std:: vector<size_t> cloneCounts;
			cloneCounts.resize((size_t)(std:: numeric_limits<HashValueType>::max()) + 1, 0);
			size_t pos = 1; 
			while (pos < seq.size() - unitLength) {
				HashValueType h = hashSeq[pos];
				if (h != 0) {
					assert(cloneCounts[h] < std::numeric_limits<size_t>::max());
					++cloneCounts[h];
					++pos;
				}
				else {
					if (pos + unitLength < hashSeq.size()) { 
						if (pos > unitLength) {
							assert(hashSeq[pos + unitLength - 1] == 0);
							pos += unitLength;
						}
						while (pos < seq.size() - unitLength && hashSeq[pos] == 0) {
							++pos;
						}
					}
					else {
						break; // while pos
					}
				}
			}
			
			cloneFragments.resize((size_t)(std:: numeric_limits<HashValueType>::max()) + 1);
			pos = 1; 
			while (pos < seq.size() - unitLength) {
				HashValueType h = hashSeq[pos];
				if (h != 0) {
					if (cloneCounts[h] >= 2) {
						cloneFragments[h].reserve(cloneCounts[h]);
						cloneFragments[h].push_back(pos);
					}
					++pos;
				}
				else {
					if (pos + unitLength < hashSeq.size()) { 
						if (pos > unitLength) {
							assert(hashSeq[pos + unitLength - 1] == 0);
							pos += unitLength;
						}
						while (pos < seq.size() - unitLength && hashSeq[pos] == 0) {
							++pos;
						}
					}
					else {
						break; // while pos
					}
				}
			}
		}
	
		ThreadQueue<std::vector<std::vector<CloneSetData> > *> que(10);
		boost::thread eater(boost::bind(&CloneDetector::send_clone_set_data_to_listener, this, &que, pListener));

		size_t worker = std::max((size_t)1, (size_t)numThreads);
		std::vector<size_t> validCis;
		validCis.reserve(numThreads);
		size_t ci = 1; 
		while (ci < cloneFragments.size()) {
			validCis.clear();
			while (ci < cloneFragments.size() && validCis.size() < worker) {
				if (cloneFragments[ci].size() > 0) {
					validCis.push_back(ci);
				}
				++ci;
			}

			std::vector<std::vector<CloneSetData> > *pFoundCloneSetsForThreads = new std::vector<std::vector<CloneSetData> >();
			std::vector<std::vector<CloneSetData> > &foundCloneSetsForThreads = *pFoundCloneSetsForThreads;
			foundCloneSetsForThreads.resize(worker);

			size_t validCiCount = validCis.size();
#pragma omp parallel for schedule(dynamic)
			for (int cii = 0; cii < validCiCount; ++cii) {
				const size_t threadNum = cii;
				size_t tci = validCis[cii];
				std::vector<CloneSetData> &foundCloneSets = foundCloneSetsForThreads[threadNum];
				foundCloneSets.clear();
				std:: vector<size_t/* pos */> &poss = cloneFragments[tci];
				if (poss.size() > 1) {
					typename SubSequence::SequencePrevComparator spc(unitLength, pSeq);
					std:: sort(poss.begin(), poss.end(), spc);
					size_t j = 0;
					while (j < poss.size()) {
						size_t pj = poss[j];
						SubSequence ssj(pj, pj + unitLength);
						SubSequence ssk;
						size_t k = j + 1;
						while (k < poss.size() && subsequenceEqual(pSeq, (ssk = SubSequence(poss[k], poss[k] + unitLength)), ssj)) {
							++k;
						}

						// here, subsequence begining at j, ..., subsequence begining at k - 1 have the same subsequence
						assert(k == poss.size() || ! subsequenceEqual(pSeq, ssj, ssk));

						size_t size = k - j;
						if (size <= 1) {
							NULL;
						}
						else {
							const ElemType &firstPrev = to_reversereference_compared(*pSeq, poss[j] - 1, poss[j], poss[j] + unitLength);
							const ElemType &lastPrev = to_reversereference_compared(*pSeq, poss[k - 1] - 1, poss[k - 1], poss[k - 1] + unitLength);
							if (firstPrev != 0 && firstPrev != -1 && firstPrev == lastPrev) { // 2007/10/29 //if (firstPrev != 0 && firstPrev == lastPrev) {
								NULL;
							}
							else {
								size_t maxExtend = calc_max_extend(poss, j, k, unitLength);
								typename SubSequence::PrevExtensionComparator pec(unitLength + maxExtend, pSeq);
								std:: sort(poss.begin() + j, poss.begin() + k, pec);
								output_clone_set(poss, j, k, unitLength + maxExtend, pListener, &foundCloneSets);
								find_clone_set_i(&poss, j, k, unitLength + maxExtend, pListener, &foundCloneSets);
							}
						}

						j = k;
						ssj = ssk;
					}
				}
			}

			que.push(pFoundCloneSetsForThreads);
		}

		que.push(NULL);
		eater.join();

		hashSeq.clear();
	}
private:
	void find_clone_set_i(std:: vector<size_t/* pos */> *pPoss, size_t begin, size_t end, 
			size_t baseLength, CloneSetListener *pListener, std::vector<CloneSetData> *pFoundCloneSets)
	{
		if (end - begin <= 1) {
			return;
		}

		std:: vector<size_t/* pos */> &poss = *pPoss;
		typename SubSequence::ExtensionPrevComparator epc(baseLength, pSeq);
		std:: sort(poss.begin() + begin, poss.begin() + end, epc);

		size_t nnBegin = begin;
		while (nnBegin < end && (poss[nnBegin] + baseLength >= (*pSeq).size() || (*pSeq)[poss[nnBegin] + baseLength] == 0)) {
			++nnBegin;
		}
		begin = nnBegin;

		if (end - nnBegin <= 1) {
			return;
		}

		size_t j = nnBegin;
		while (j < end) {
			size_t k = j + 1;
			while (k < end && to_compared(*pSeq, poss[k] + baseLength, poss[k]) == to_compared(*pSeq, poss[j] + baseLength, poss[j])) {
				++k;
			}
			//std::cerr << to_compared(*pSeq, poss[j] + baseLength, poss[j]) << ": "; for (size_t I = j; I < k; ++I) { std::cerr << poss[I] << " "; } std::cerr << std::endl;

			// here, subsequence begining at j, ..., subsequence begining at k - 1 have the same subsequence
			assert(k == end || ! (to_compared(*pSeq, poss[k] + baseLength, poss[k]) == to_compared(*pSeq, poss[j] + baseLength, poss[j])));

			size_t size = k - j;
			if (size <= 1) {
				NULL;
			}
			else {
				size_t pj = poss[j];
				const ElemType &firstPrev = to_reversereference_compared(*pSeq, pj - 1, pj, pj + baseLength);
				const ElemType &lastPrev = to_reversereference_compared(*pSeq, poss[k - 1] - 1, poss[k - 1], poss[k - 1] + baseLength);
				if (firstPrev != 0 && firstPrev != -1 && firstPrev == lastPrev) { // 2007/11/02 //if (firstPrev != 0 && firstPrev == lastPrev) {
					NULL;
				}
				else {
					size_t maxExtend = calc_max_extend(poss, j, k, baseLength);
					typename SubSequence::PrevExtensionComparator pec(baseLength + maxExtend, pSeq);
					std:: sort(poss.begin() + j, poss.begin() + k, pec);
					output_clone_set(poss, j, k, baseLength + maxExtend, pListener, pFoundCloneSets);
					find_clone_set_i(&poss, j, k, baseLength + maxExtend, pListener, pFoundCloneSets);
				}
			}

			j = k;
		}
	}
	void output_clone_set(const std:: vector<size_t/* pos */> &poss, size_t begin, size_t end, size_t baseLength, CloneSetListener *pListener, 
			std::vector<CloneSetData> *pFoundCloneSets)
	{
		if (end - begin == 0 || ! (*pListener).codeCheck(poss[begin], baseLength)) {
			return;
		}

		std:: vector<CloneSetItem> cloneSet;

		size_t p = begin;
		while (p < end) {
			const ElemType &prevp = to_reversereference_compared(*pSeq, poss[p] - 1, poss[p], poss[p] + baseLength);
			size_t q = p + 1;
			while (q < end && to_reversereference_compared(*pSeq, poss[q] - 1, poss[q], poss[q] + baseLength) == prevp) {
				++q;
			}

			// here, subsequence begining at p, ..., subsequence begining at q - 1 have the same prev
			assert(q == end || prevp != to_reversereference_compared(*pSeq, poss[q] - 1, poss[q], poss[q] + baseLength));

			size_t i = p;
			while (i < q) {
				const ElemType &extensioni = to_compared(*pSeq, poss[i] + baseLength, poss[i]);
				size_t j = i + 1;
				while (j < q && to_compared(*pSeq, poss[j] + baseLength, poss[j]) == extensioni) {
					++j;
				}
				
				// here, subsequence begining at i, ..., subsequence begining at j - 1 have the same extension
				assert(j == q || extensioni != to_compared(*pSeq, poss[j] + baseLength, poss[j]));
				
				cloneSet.resize(cloneSet.size() + 1);
				CloneSetItem &cs = cloneSet.back();
				cs.prev = prevp;
				cs.extension = extensioni;
				cs.poss.insert(cs.poss.end(), poss.begin() + i, poss.begin() + j);

				i = j;
			}

			p = q;
		}
		
		if ((*pListener).rangeCheck(cloneSet)) {
			std::vector<CloneSetData> &foundCloneSets = *pFoundCloneSets;
			foundCloneSets.resize(foundCloneSets.size() + 1);
			CloneSetData &cloneSetData = foundCloneSets.back();
			cloneSetData.cloneSet.swap(cloneSet);
			cloneSetData.baseLength = baseLength;
		}
	}
	size_t calc_max_extend(const std:: vector<size_t/* pos */> &poss, size_t begin, size_t end, size_t baseLength)
	{
		assert(end - begin >= 2);

		size_t extend = 0;
		while (true) {
			size_t posj = poss[begin];
			const ElemType &ej = to_compared(*pSeq, posj + baseLength + extend, posj);
			if (ej == 0) {
				return extend;
			}
			for (size_t p = begin + 1; p < end; ++p) {
				size_t pos = poss[p];
				const ElemType &ep = to_compared(*pSeq, pos + baseLength + extend, pos);
				if (ep != ej) {
					return extend;
				}
			}
			++extend;
		}
		return extend;
	}
	void calc_hash_seq(SequenceHashFunction &hashFunc)
	{
		const std::vector<ElemType> &seq = *pSeq;
		
		hashSeq.clear();
		hashSeq.resize(seq.size(), 0);
		
		size_t num = bottomUnitLength * multiply;
		std:: vector<size_t> factors0;
		factorize(&factors0, num);
		if (factors0.size() == 0) {
			make_bottom_level_hash_sequence(seq, hashFunc, &hashSeq, bottomUnitLength * multiply);
		}
		else {
			size_t beginPos = 0;
			assert(seq.size() == 0 || seq.back() == 0);
	
			while (beginPos < seq.size() - 1) {
				typename std::vector<ElemType>::const_iterator j = std::find(seq.begin() + beginPos + 1, seq.end(), 0);
				size_t nextPos = j - seq.begin();
				assert(seq[nextPos] == 0);
				size_t endPos = nextPos + 1;
				assert(endPos <= seq.size());
				
				size_t blockSize = endPos - beginPos;
				if (blockSize < num) {
					// hashSeq[beginPos ... endPos] has been zero-filled already.
				}
				else {
					int fi = factors0.size() - 1;
					size_t f = factors0[fi];
					make_bottom_level_hash_sequence(seq, hashFunc, &hashSeq, f, beginPos, endPos);
					
					size_t curUnitLength = f;
					while (--fi >= 0) {
						size_t f = factors0[fi];
						multiple_hash_sequence(hashFunc, &hashSeq, curUnitLength, f, beginPos, endPos);
						curUnitLength *= f;
					}
					assert(curUnitLength == bottomUnitLength * multiply);
				}
				beginPos = nextPos;
			}
		}
	}
private:
	static inline void multiple_hash_sequence(SequenceHashFunction &hashFunc, 
			std:: vector<HashValueType> *pHashSeq, size_t unitLength, size_t multiply)
	{
		std:: vector<HashValueType> &hashSeq = *pHashSeq;
		if (! hashSeq.empty()) {
			multiple_hash_sequence(hashFunc, pHashSeq, unitLength, multiply, 0, hashSeq.size());
		}
	}
	static void multiple_hash_sequence(SequenceHashFunction &hashFunc, 
			std:: vector<HashValueType> *pHashSeq, size_t unitLength, size_t multiply, size_t beginPos, size_t endPos)
	{
		std:: vector<HashValueType> &hashSeq = *pHashSeq;
		assert(beginPos < hashSeq.size());
		assert(hashSeq[beginPos] == 0);
		assert(endPos <= hashSeq.size());
		assert(hashSeq[endPos - 1] == 0);
		assert(unitLength >= 1);
		
		for (int i = beginPos + 1; i < endPos - unitLength * multiply; ++i) {
			HashValueType value = 0;
			for (size_t j = 0; j < multiply; ++j) {
				HashValueType h = hashSeq[i + j * unitLength];
				/* 
				** when i == endPos - unitLength * multiply - 1 and j == multiply - 1,
				** i + j * unitLength 
				**   = endPos - unitLength * multiply - 1 + unitLength * (multiply - 1)
				**   = endPos - unitLength * multiply - 1 + unitLength * multiply - unitLength
				**   = endPos - unitLength - 1
				** Therefore, i + j * unitLength < endPos - 1
				*/
				assert(h != 0);
				value += h;
			}
			hashSeq[i] = value == 0 ? 1 : value; // 0はdelimiterと見なされるため、ハッシュ値として用いることはできない
		}
		std::fill(hashSeq.begin() + endPos - unitLength * multiply, hashSeq.begin() + endPos, 0);
	}
	static inline void make_bottom_level_hash_sequence(const std:: vector<ElemType> &seq, SequenceHashFunction &hashFunc, 
			std:: vector<HashValueType> *pHashSeq, size_t unitLength)
	{
		std:: vector<HashValueType> &hashSeq = *pHashSeq;

		if (! hashSeq.empty()) {
			make_bottom_level_hash_sequence(seq, hashFunc, pHashSeq, unitLength, 0, seq.size());
		}
	}
	static void make_bottom_level_hash_sequence(const std:: vector<ElemType> &seq, SequenceHashFunction &hashFunc, 
			std:: vector<HashValueType> *pHashSeq, size_t unitLength, size_t beginPos, size_t endPos)
	{
		assert(beginPos < seq.size());
		assert(seq[beginPos] == 0);
		assert(endPos <= seq.size());
		assert(seq[endPos - 1] == 0);
		assert(unitLength >= 1);

		std:: vector<HashValueType> &hashSeq = *pHashSeq;
		assert(hashSeq.size() == seq.size());

		size_t i = beginPos + 1;
		if (endPos - beginPos >= unitLength) {
			typename std::vector<ElemType>::const_iterator range_begin = seq.begin() + beginPos + 1;
			typename std::vector<ElemType>::const_iterator range_end = seq.begin() + endPos - unitLength; // value of i at the last repetition of the following 'for' loop
			//assert(std::find(range_begin, range_end, 0) == range_end);
			for (; i < endPos - unitLength; ++i) {
				HashValueType hashValue = hashFunc(seq, i, i + unitLength);
				hashSeq[i] = hashValue == 0 ? 1 : hashValue; // 0はdelimiterと見なされるため、ハッシュ値として用いることはできない
			}
		}
		std::fill(hashSeq.begin() + i, hashSeq.begin() + endPos, 0);
	}

	static void factorize(std:: vector<size_t> *pFactors, size_t number0)
	{
		std:: vector<size_t> &factors = *pFactors;
		factors.clear();

		size_t number = number0;
		while (number > 1) {
			bool found = false;
			for (size_t i = 2; i < number / 2; ++i) {
				if (number % i == 0) {
					factors.push_back(i);
					number /= i;
					found = true;
					break; // for i
				}
			}
			if (! found) {
				factors.push_back(number);
				return;
			}
		}
	}

	//static void fill_zero(const std:: vector<ElemType> &seq, std:: vector<HashValueType> *pHashSeq, size_t unitLength)
	//{
	//	assert(seq.size() == (*pHashSeq).size());
	//	
	//	if ((*pHashSeq).size() == 0) {
	//		return;
	//	}
	//	size_t i = (*pHashSeq).size() - 1; 
	//	while (true) {
	//		if (seq[i] == 0) {
	//			for (size_t j = 0; j < unitLength; ++j) {
	//				(*pHashSeq)[i] = 0;
	//				if (--i == 0) {
	//					return;
	//				}
	//			}
	//		}
	//		if (--i == 0) {
	//			return;
	//		}
	//	}
	//}
};

#endif // CLONEDETECTOR_H

