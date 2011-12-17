#if ! defined ALLOCAARRAY_H

#include <cassert>
#include <stdexcept>

#if defined _MSC_VER

#include <malloc.h>

#define DECL_ALLOCA_ARRAY(variable, ElemType, count) alloca_array<ElemType> variable(_alloca(count * sizeof(ElemType)), (count))

template<typename ElemType>
class alloca_array {
public:
    typedef ElemType value_type;
    typedef ElemType *iterator;
    typedef const ElemType *const_iterator;
    typedef ElemType &reference;
    typedef const ElemType &const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
private:
	ElemType *buf;
	size_type buf_size;
public:
	alloca_array(void *alloca_buffer, size_t count)
		: buf(NULL), buf_size(0)
	{
		buf = reinterpret_cast<ElemType *>(alloca_buffer);
		for (size_t i = 0; i < count; ++i) {
			new(&buf[i]) ElemType();
		}
		buf_size = count;
	}
	~alloca_array()
	{
		for (size_t i = buf_size; i-- > 0; ) {
			buf[i].~ElemType();
		}
	}
	reference operator[](size_t index)
	{
		return buf[index];
	}
	const_reference operator[](size_t index) const
	{
		return buf[index];
	}
	iterator begin()
	{
		return &buf[0];
	}
	iterator end()
	{
		return &buf[buf_size];
	}
	const_iterator begin() const
	{
		return &buf[0];
	}
	const_iterator end() const
	{
		return &buf[buf_size];
	}
	
	// swap (note: linear complexity)
	void swap (alloca_array<ElemType> &y) {
		std::swap_ranges(begin(), end(), y.begin());
	}

	// direct access to data (read-only)
	const ElemType* data() const 
	{ 
		return buf; 
	}

	// use array as C array (direct read/write access to data)
	ElemType *c_array() 
	{ 
		return buf; 
	}

	// assignment with type conversion
	template <typename T2>
	alloca_array<ElemType> &operator= (const alloca_array<T2> &rhs) {
		std::copy(rhs.begin(), rhs.end(), begin());
		return *this;
	}

	size_type size() const
	{
		return buf_size;
	}
	bool empty() const
	{
		return buf_size > 0;
	}
	reference at(size_type i) 
	{ 
		rangecheck(i); 
		return buf[i]; 
	}
	const_reference at(size_type i) const 
	{ 
		rangecheck(i); 
		return buf[i]; 
	}
	void rangecheck (size_type i) const
	{
		if (i >= buf_size) { 
			throw std::range_error("alloca_array<>: index out of range");
		}
	}
};

#endif // _MSC_VER

#if defined __GNUC__

#include <vector>

#define DECL_ALLOCA_ARRAY(variable, ElemType, count) alloca_array<ElemType> variable(count)

template<typename ElemType>
class alloca_array 
	: public std::vector<ElemType> 
{
public:
	typedef typename std::vector<ElemType>::size_type size_type;

	alloca_array(size_t count)
		: std::vector<ElemType>(count)
	{}
	
	// direct access to data (read-only)
	const ElemType* data() const 
	{ 
		return &(*this)[0];
	}

	// use array as C array (direct read/write access to data)
	ElemType *c_array() 
	{ 
		return &(*this)[0];
	}

	// assignment with type conversion
	template <typename T2>
	alloca_array<ElemType> &operator= (const alloca_array<T2> &rhs) {
		std::copy(rhs.begin(), rhs.end(), begin());
		return *this;
	}

	void rangecheck (size_type i) const
	{
		if (i >= this->size()) { 
			throw std::range_error("alloca_array<>: index out of range");
		}
	}
};

#endif // __GNUC__

#endif // ALLOCAARRAY_H
