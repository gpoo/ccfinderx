#if ! defined __THREADQUEUE_H__
#define __THREADQUEUE_H__

#include <boost/thread.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION >= 103600
#include <boost/thread/condition.hpp>
#endif

#include <boost/utility.hpp>
#include <queue>

template<typename ElementType>
class ThreadQueue : private boost::noncopyable {
private:
	boost::mutex mt;
#if BOOST_VERSION >= 103600
	boost::condition_variable_any waitingEmpty;
	boost::condition_variable_any waitingElement;
#else
	boost::condition waitingEmpty;
	boost::condition waitingElement;
#endif
	size_t limitSize;
	std::queue<ElementType> body;
private:
	typedef boost::mutex::scoped_lock lock;
public:
	ThreadQueue(size_t limitSize_)
		: body(), limitSize(limitSize_)
	{
		assert(limitSize >= 1);
	}
	void push(const ElementType &element)
	{
		lock lk(mt);
		while (true) {
			if (body.size() < limitSize) {
				body.push(element);
				waitingElement.notify_one();
				return;
			}
			waitingEmpty.wait(lk);
		}
	}
	ElementType pop()
	{
		lock lk(mt);
		while (true) {
			if (body.size() > 0) {
				ElementType element = body.front();
				body.pop();
				waitingEmpty.notify_one();
				return element;
			}
			waitingElement.wait(lk);
		}
	}
};

#endif // __THREADQUEUE_H__
