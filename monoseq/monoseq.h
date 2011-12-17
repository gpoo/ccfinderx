#if ! defined MONOSEQ_H__
#define MONOSEQ_H__

#include <vector>

class monoseq {
private:
	std::vector<int> tvec;
	std::vector<int> ivec;
public:
	class error {
	};
	class no_monotonous : public error {
	};
	class out_of_range : public error {
	};
public:
	monoseq();
	monoseq(const monoseq &rhs);
	monoseq &operator=(const monoseq &rhs);
	virtual ~monoseq();
	void push_back(int value) throw(no_monotonous);
	int at(int index) const throw(out_of_range);
};

#endif // MONOSEQ_H__
