#include <cassert>
#include <cstdlib>
#include <map>
#include <hash_map>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "clonedetector.h"

class MySequenceHashFunction : public CloneDetector<char, unsigned short>::SequenceHashFunction {
public:
	virtual unsigned short operator()(std:: vector<char>::const_iterator beginPos, std:: vector<char>::const_iterator endPos)
	{
		unsigned short hashValue = 0;
		int j = 0;
		for (std:: vector<char>::const_iterator i = beginPos; i != endPos; ++i) {
			hashValue += (unsigned short)(*i) * (*i);
			//hashValue += (j + 1) * (unsigned short)(*i);
			//hashValue += (1 + 5 * j) * ((unsigned short)(*i));
			//hashValue *= 2; hashValue += (unsigned short)*i;
			++j;
		}
		return hashValue;
	}
};

class MyCloneSetListener : public CloneDetector<char, unsigned short>::CloneSetListener {
public:
	void found(const stdext:: hash_map<char/* prev */, std:: vector<size_t/* pos */> > &sPrevPoss)
	{
		const size_t unitLength = getUnitLength();
		const std:: vector<char> &seq = refSeq();

		std:: cout << "begin" << std:: endl;
		for (stdext:: hash_map<char/* prev */, std:: vector<size_t/* pos */> >::const_iterator pi = sPrevPoss.begin(); 
				pi != sPrevPoss.end(); ++pi)
		{
			if (pi->first == 0) {
				std:: cout << "prev: delimiter" << std:: endl;
			}
			else {
                std:: cout << "prev: " << (char)pi->first << std:: endl;
			}
			const std:: vector<size_t/* pos */> &poss = pi->second;
			for (size_t si = 0; si < poss.size(); ++si) {
				size_t pos0 = poss[si];
				std:: cout << "pos: " << pos0 << " seq:";
				for (size_t j = 0; j < unitLength; ++j) {
					std:: cout << " " << (char)seq[pos0 + j];
				}
				std:: cout << std:: endl;
			}
		}
		std:: cout << "end" << std:: endl;
	}
};

class MyClonePairListener : public CloneDetector<char, unsigned short>::ClonePairListener {
public:
	void found(size_t posA, size_t posB, size_t length)
	{
		const std:: vector<char> &seq = refSeq();

		std:: cout << "pair" << std:: endl;
		std:: cout << "length: " << length << std:: endl;
		std:: cout << "pos: " << posA << " seq:";
		{
			for (size_t j = 0; j < length; ++j) {
				std:: cout << " " << (char)seq[posA + j];
			}
		}
		std:: cout << std:: endl;
		std:: cout << "pos: " << posB << " seq:";
		{
			for (size_t j = 0; j < length; ++j) {
				std:: cout << " " << (char)seq[posB + j];
			}
		}
		std:: cout << std:: endl;
	}
};

int main(int argc, char *argv[])
{
	const int inputSize = 20000;

	// make input sequence
	std:: vector<char> seq;
	{
		seq.reserve(inputSize + 2);
		seq.push_back(0);
		for (size_t i = 0; i < inputSize; ++i) {
			int r = rand();
			seq.push_back('a' + r % 3);
		}
		for (size_t i = 0; i < inputSize; i += 209) {
			seq[i] = 0;
		}
		seq.push_back(0);
	}

	CloneDetector<char, unsigned short> cd;
	cd.attachSequence(&seq);
	cd.setBottomUnitLength(3);
	cd.setMultiply(4);
	size_t unitLength = cd.getUnitLength();
	MySequenceHashFunction hashFunc;
	MyClonePairListener lis2;
	cd.findClonePair(&lis2, hashFunc);

	return 0;
}

