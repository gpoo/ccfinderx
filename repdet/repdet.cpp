// repdet.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <cassert>
#include <vector>
#include <map>

#include "repdet.h"

using namespace repdet;

#if defined REPDET_TEST_MAIN

#include <iostream>
#include <cstdlib>
#include <boost/dynamic_bitset.hpp>
#include <boost/timer.hpp>

void print_repetitions(const repdet::MapRepposRepitition &reps, const std:: vector<char> &data)
{
	boost::dynamic_bitset<> b;
	b.resize(data.size());
	
	std::vector<repdet::reppos> keys;
	keys.reserve(reps.size());
	for (repdet::MapRepposRepitition::const_iterator i = reps.begin(); i != reps.end(); ++i) {
		keys.push_back(i->first);
	}

	for (size_t i = 0; i < keys.size(); ++i) {
		const repdet::reppos &key = keys[i];
		const Repetition &rep = reps.find(key)->second;
		for (size_t pos = rep.beginEnd.first + rep.unit; pos < rep.beginEnd.second; ++pos) {
			b.set(pos, true);
		}
	}
	bool inRepetition = false;
	for (size_t i = 0; i < data.size(); ++i) {
		if (! inRepetition) {
			if (b.test(i)) {
				inRepetition = true;
				std:: cout << "(";
			}
		}
		else {
			if (! b.test(i)) {
				inRepetition = false;
				std:: cout << ")";
			}
		}
		std:: cout << data[i];
	}
	if (inRepetition) {
		inRepetition = false;
		std:: cout << ")";
	}
	std:: cout << std:: endl;
}

void print_repetitions_diff(const repdet::MapRepposRepitition &reps, 
		const repdet::MapRepposRepitition &repsS, 
		const std:: vector<char> &data)
{
	if (repsS.size() != reps.size()) {
		std:: cout << "reps.size() = " << reps.size() << std:: endl;
		std:: cout << "repsS.size() = " << repsS.size() << std:: endl;
		
		std::vector<repdet::reppos> keys;
		keys.reserve(reps.size());
		for (repdet::MapRepposRepitition::const_iterator i = reps.begin(); i != reps.end(); ++i) {
			keys.push_back(i->first);
		}
		std::vector<repdet::reppos> keysS;
		keysS.reserve(repsS.size());
		for (repdet::MapRepposRepitition::const_iterator i = repsS.begin(); i != repsS.end(); ++i) {
			keysS.push_back(i->first);
		}

		std::vector<repdet::reppos>::const_iterator ki = keys.begin();
		std::vector<repdet::reppos>::const_iterator kiS = keysS.begin();
		while (ki != keys.end() && kiS != keysS.end()) {
			repdet::MapRepposRepitition::const_iterator leftI = reps.find(*ki);
			repdet::MapRepposRepitition::const_iterator rightI = repsS.find(*kiS);
			if (leftI->first == rightI->first) {
				++leftI;
				++rightI;
			}
			else if (leftI->first < rightI->first) {
				const Repetition &rep = leftI->second;
				std:: cout << "left: " << rep.beginEnd.first << ", " << rep.beginEnd.second << ", " << rep.unit << std:: endl;
				++leftI;
			}
			else {
				const Repetition &rep = rightI->second;
				std:: cout << "right: " << rep.beginEnd.first << ", " << rep.beginEnd.second << ", " << rep.unit << std:: endl;
				++rightI;
			}
		}
	}
}

#if 0
int main(int argc, char *argv[])
{
	size_t upperLimit = 0;

	// prepare data
	std:: vector<char> data;
	data.resize(3000);
	for (size_t i = 0; i < data.size(); ++i) {
		data[i] = rand() % 3 + 'a';
	}

	//// print data
	//std:: cout << "raw data:" << std:: endl;
	//for (size_t i = 0; i < data.size(); ++i) {
	//	std:: cout << data[i];
	//}
	//std:: cout << std:: endl;

	std::cout << "input random string (length = " << data.size() << "): " << std::endl;
	
	boost::timer timer;
	double mark1 = timer.elapsed();

	// find repetation
	repdet::MapRepposRepitition reps;
	RepetitionDetector<char>().findRepetitions(&reps, data, upperLimit);

	double mark2 = timer.elapsed();

	print_repetitions(reps, data);

	//// print repetitions
	//std:: cout << "found repetitions:" << std:: endl;
	//for (std:: map<std:: pair<size_t/* begin */, size_t/* end */>, Repetition>::const_iterator i = reps.begin(); i != reps.end(); ++i) {
	//	const Repetition &rep = i->second;
	//	std:: cout << rep.beginEnd.first << ", " << rep.beginEnd.second << ", " << rep.unit << std:: endl;
	//	for (size_t i = rep.beginEnd.first; i < rep.beginEnd.second; ++i) {
	//		std:: cout << data[i];
	//	}
	//	std:: cout << std:: endl;
	//}

	std:: cout << "consumed time for detecting repetitions = " << (mark2 - mark1) << " seconds" << std:: endl;

	return 0;
}
#else
int main(int argc, char *argv[])
{
	size_t upperLimit = 0;
	size_t N = 20;

	std:: vector<char> data;
	{
		std::vector<std::vector<char> > fibo;
		fibo.resize(N);
		fibo[0].push_back('b');
		fibo[1].push_back('a');
		for (size_t i = 2; i < fibo.size(); ++i) {
			fibo[i].insert(fibo[i].end(), fibo[i - 1].begin(), fibo[i - 1].end());
			fibo[i].insert(fibo[i].end(), fibo[i - 2].begin(), fibo[i - 2].end());
		}
		data.swap(fibo.back());
	}

	std::cout << "input fibo(" << N << "): " << std::endl;

	boost::timer timer;
	double mark1 = timer.elapsed();

	// find repetation
	repdet::MapRepposRepitition reps;
	RepetitionDetector<char>().findRepetitions(&reps, data, upperLimit);

	double mark2 = timer.elapsed();

	print_repetitions(reps, data);

	//// print repetitions
	//std:: cout << "found repetitions:" << std:: endl;
	//for (std:: map<std:: pair<size_t/* begin */, size_t/* end */>, Repetition>::const_iterator i = reps.begin(); i != reps.end(); ++i) {
	//	const Repetition &rep = i->second;
	//	std:: cout << rep.beginEnd.first << ", " << rep.beginEnd.second << ", " << rep.unit << std:: endl;
	//	for (size_t i = rep.beginEnd.first; i < rep.beginEnd.second; ++i) {
	//		std:: cout << data[i];
	//	}
	//	std:: cout << std:: endl;
	//}

	std:: cout << "consumed time for detecting repetitions = " << (mark2 - mark1) << " seconds" << std:: endl;
}
#endif


#endif // REPDET_TEST_MAIN

