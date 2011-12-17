#if ! defined __BITVECTOR_H__
#define __BITVECTOR_H__

#include <vector>

class bitvector {
private:
	size_t num_bits;
	std:: vector<unsigned char> body;
public:
	inline ~bitvector() { }
	inline bitvector() : num_bits(0), body() { }
	inline bitvector(const bitvector& rhs) : num_bits(rhs.num_bits), body(rhs.body) { }
	void resize(size_t numBits, bool defaultValue = false);
	inline size_t size() const { return num_bits; }
	void reserve(size_t numBits);
	inline size_t capacity() const { return body.capacity() * 8; }
	void set(size_t index, bool value);
	void set(size_t begin, size_t end, bool value);
	bool test(size_t index) const;
	bitvector& operator=(const bitvector& rhs);
	inline void clear() { resize(0); }
	size_t countValue(size_t begin, size_t end, bool value) const;
	inline size_t countTrue(size_t begin, size_t end) const { return countValue(begin, end, true); }
	inline size_t countFalse(size_t begin, size_t end) const { return countValue(begin, end, false); }
	void swap(bitvector &rhs);
};

namespace std {

template<> 
inline void swap(bitvector &left, bitvector &right) { left.swap(right); }

} // namespace

#endif // __BITVECTOR_H__
