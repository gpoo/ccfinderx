#include <cassert>
#include "bitvector.h"

void bitvector::resize(size_t numBits, bool defaultValue)
{
	size_t numBytes = (numBits + 8 - 1) / 8;
	body.resize(numBytes);
	size_t old_num_bits = num_bits;
	num_bits = numBits;
	if (numBits > old_num_bits) {
		size_t n = (old_num_bits + 8 - 1) / 8 * 8;
		assert(old_num_bits <= n && n % 8 == 0);
		set(old_num_bits, n, defaultValue);
		for (size_t i = n / 8 + 1; i < numBytes; ++i) {
			body[i] = defaultValue ? 0xff:0x00;
		}
	}
}

void bitvector::reserve(size_t numBits)
{
	size_t numBytes = (numBits + 8 - 1) / 8;
	body.reserve(numBytes);
}

void bitvector::set(size_t index, bool value)
{
	assert(0 <= index);
	assert(index < num_bits);
	size_t byte_address = index / 8;
	assert(byte_address < body.size());
	size_t bit_address = index % 8;
	unsigned char mask = 1 << bit_address;
	if (value) {
		// set the bit
		body[byte_address] |= mask;
	}
	else {
		// clear the bit
		body[byte_address] &= ~mask;
	}
}

void bitvector::set(size_t begin, size_t end, bool value)
{
	if (begin < 0) {
		begin = 0;
	}
	if (end > num_bits) {
		end = num_bits;
	}
	if (begin >= end) {
		return;
	}

	size_t index = begin;
	while (index < end) {
		size_t byte_address = index / 8;
		assert(byte_address < body.size());
		size_t bit_address = index % 8;
		if (bit_address == 0 && index + 8 <= end) {
			body[byte_address] = value ? 0xff : 0x00;
			index += 8;
		}
		else {
			unsigned char mask = 1 << bit_address;
			if (value) {
				// set the bit
				body[byte_address] |= mask;
			}
			else {
				// clear the bit
				body[byte_address] &= ~mask;
			}
			++index;
		}
	}
}

bool bitvector::test(size_t index) const
{
	assert(index < num_bits);
	size_t byte_address = index / 8;
	assert(byte_address < body.size());
	size_t bit_address = index % 8;
	unsigned char mask = 1 << bit_address;
	if (body[byte_address] & mask) {
		return true;
	}
	else {
		return false;
	}
}

bitvector& bitvector::operator=(const bitvector& rhs)
{
	if (&rhs == this) {
		return *this;
	}

	body = rhs.body;
	num_bits = rhs.num_bits;

	return *this;
}

size_t bitvector::countValue(size_t begin, size_t end, bool value) const
{
	static const char bit2count[256] = {
#include "bitcounttable.h"
	};

	size_t count = 0;
	if (begin < 0) {
		begin = 0;
	}
	if (end > num_bits) {
		end = num_bits;
	}
	if (begin > end) {
		assert(false);
		return 0;
	}

	size_t index = begin; 
	while (index < end) {
		size_t byte_address = index / 8;
		assert(byte_address < body.size());
		size_t bit_address = index % 8;
		if (bit_address == 0 && index + 8 <= end) {
			unsigned char pat = body[byte_address];
			count += bit2count[pat];
			index += 8;
		}
		else {
			unsigned char mask = 1 << bit_address;
			if (body[byte_address] & mask) {
				++count;
			}
			++index;
		}
	}
	
	if (value) {
        return count;
	}
	else {
		return (end - begin) - count;
	}
}

void bitvector::swap(bitvector &rhs)
{
	if (&rhs == this) {
		return;
	}

	size_t tmp = this->num_bits; this->num_bits = rhs.num_bits; rhs.num_bits = tmp;
	this->body.swap(rhs.body);
}

