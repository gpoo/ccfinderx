#include "threadqueue.h"

#include <iostream>
#include <string>

#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>

#include <boost/random.hpp>
#include <boost/format.hpp>

void sleep(size_t nanoSec)
{
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.nsec += nanoSec;
	boost::thread::sleep(xt);
}

boost::variate_generator<boost::mt19937, boost::uniform_smallint<> > generate_rand(int seed, int bottom, int top)
{
	boost::mt19937 gen( static_cast<unsigned long>(seed) );
	boost::uniform_smallint<> dst(bottom, top);

	return boost::variate_generator<
		boost::mt19937, boost::uniform_smallint<>
	>(gen, dst);
}

void messageGenerator(ThreadQueue<std::string> *pQueue, int seed, size_t genCount)
{
	boost::variate_generator<boost::mt19937, boost::uniform_smallint<> > rand = generate_rand(seed, 0, 100000000);

	for (size_t i = 0; i < genCount; ++i) {
		sleep(rand());
		std::string message = (boost::format("%d: %03d") % seed % (i + 1)).str();
		(*pQueue).push(message);
	}
}

int main(int argc, char *argv[])
{
	size_t size1 = 50;
	size_t size2 = 50;

	ThreadQueue<std::string> que(4);
	boost::thread gen1(boost::bind(messageGenerator, &que, 1, size1));
	boost::thread gen2(boost::bind(messageGenerator, &que, 2, size2));

	for (size_t count = 0; count < size1 + size2; ++count) {
		std::string message = que.pop();
		std::cout << message << std::endl;
	}

	// here, gen1 and gen2 have been terminated.
	gen1.join();
	gen2.join();

	return 0;
}
