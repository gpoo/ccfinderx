#if ! defined __BITVECTOR_H__
#define __BITVECTOR_H__

#include <vector>

class bitvector {
private:
	std::size_t num_bits;
	std:: vector<unsigned char> body;
public:
	inline ~bitvector() { }
	inline bitvector() : num_bits(0), body() { }
	inline bitvector(const bitvector& rhs) : num_bits(rhs.num_bits), body(rhs.body) { }
	void resize(std::size_t numBits, bool defaultValue = false);
	inline std::size_t size() const { return num_bits; }
	void reserve(std::size_t numBits);
	inline std::size_t capacity() const { return body.capacity() * 8; }
	void set(std::size_t index, bool value);
	void set(std::size_t begin, std::size_t end, bool value);
	bool test(std::size_t index) const;
	bitvector& operator=(const bitvector& rhs);
	inline void clear() { resize(0); }
	std::size_t countValue(std::size_t begin, std::size_t end, bool value) const;
	inline std::size_t countTrue(std::size_t begin, std::size_t end) const { return countValue(begin, end, true); }
	inline std::size_t countFalse(std::size_t begin, std::size_t end) const { return countValue(begin, end, false); }
	void swap(bitvector &rhs);
};

namespace std {

template<> 
inline void swap(bitvector &left, bitvector &right) { left.swap(right); }

} // namespace

#endif // __BITVECTOR_H__
