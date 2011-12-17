#include <vector>
#include <algorithm>
#include <cassert>

#include "monoseq.h"

monoseq::monoseq()
{
}

monoseq::monoseq(const monoseq &rhs)
:
	tvec(rhs.tvec), ivec(rhs.ivec)
{
}

monoseq &monoseq::operator=(const monoseq &rhs)
{
	if (&rhs == this) {
		return *this;
	}
	tvec = rhs.tvec;
	ivec = rhs.ivec;

	return *this;
}

monoseq::~monoseq()
{
}

void monoseq::push_back(int value)
{
	assert(tvec.size() == ivec.size());
	if (tvec.size() == 0) {
		tvec.push_back(value);
		ivec.push_back(0);
	}
	else {
		int last_value = tvec[tvec.size() - 1];
		if (value > last_value) {
			tvec.push_back(value);
			ivec.push_back(ivec[ivec.size() - 1] + 1);
		}
		else if (value == last_value) {
			++ivec[ivec.size() - 1];
		}
		else {
			throw no_monotonous();
		}
	}
}

int monoseq::at(int index) const
{
	assert(tvec.size() == ivec.size());
	if (! (0 <= index && index <= ivec[ivec.size() - 1])) {
		throw out_of_range();
	}

	std::vector<int>::const_iterator p = std::lower_bound(ivec.begin(), ivec.end(), index);
	int ti = std::distance(ivec.begin(), p);
	assert(0 <= ti && ti < tvec.size());
	
	return tvec[ti];
}

#if defined MONOSEQ_TEST

#include <cstdlib>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	vector<int> v;
	monoseq ms;
	int value = 0;
	for (int i = 0; i < 500; ++i) {
		if (rand() % 5 == 0) {
			++value;
		}
		v.push_back(value);
		ms.push_back(value);
	}
	
	for (i = 0; i < v.size(); ++i) {
		int vi = v[i];
		int mi = ms.at(i);
		if (vi != mi) {
			assert(0);
			cout << "error";
			return 1;
		}
	}

	return 0;
}

#endif // MONOSEQ_TEST
