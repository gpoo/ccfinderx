#if ! defined specialstringmap_h
#define specialstringmap_h

#include <vector>
#include <map>
#include <string>
#include <cassert>

# include <boost/iterator/iterator_facade.hpp>

#include "hash_map_includer.h"

#if defined NO_HASH_THINGS

template<typename DataType>
struct special_string_map : public std::map<std::string, DataType> {
public:
	void reserve_length(size_t length)
	{
	}
};

#else // NO_HASH_THINGS

template<typename DataType>
struct special_string_map {
public:
	typedef std::string key_type;
	typedef DataType data_type;
	typedef std::pair<const key_type, data_type> value_type;
	typedef typename HASH_MAP<std::string, DataType>::size_type size_type;

private:
	typedef typename HASH_MAP<std::string, DataType> BodySliceType;
	typedef typename std::vector<BodySliceType> BodyType;

private:
	BodyType body;
	size_type countOfData;

public:
	special_string_map()
		: body(), countOfData(0)
	{
	}
	special_string_map(const special_string_map<DataType> &right)
		: body(right.body), countOfData(right.countOfData)
	{
	}
	void swap(special_string_map<DataType> &right)
	{
		if (&right != this) {
			this->body.swap(right.body);
			std::swap(this->countOfData, right.countOfData);
		}
	}

public:
	class iterator : public boost::iterator_facade<iterator, value_type, boost::forward_traversal_tag, 
		typename std::pair<const std::string, DataType> &> {
	private:
		friend class boost::iterator_core_access;
		friend class const_iterator;
	public:
		BodyType *pBody;
		size_t curLength;
		typename BodySliceType::iterator it;
	public:
		iterator(const iterator &right)
			: pBody(right.pBody), curLength(right.curLength), it(right.it)
		{
		}
		iterator()
			: pBody(NULL), curLength(0), it()
		{
		}
		iterator(BodyType *pBody_, size_t curLength_, typename BodySliceType::iterator it_)
			: pBody(pBody_), curLength(curLength_), it(it_)
		{
		}
		static iterator beginIterator(BodyType *pBody_)
		{
			iterator i;
			i.pBody = pBody_;
			BodyType &body = *i.pBody;
			while (i.curLength < body.size()) {
				BodySliceType &bodyCurLength = body[i.curLength];
				if (! bodyCurLength.empty()) {
					i.it = bodyCurLength.begin();
					return i;
				}
				++i.curLength;
			}
			assert(i.curLength == body.size());
			i.pBody = NULL; // mark of end
			return i;
		}
		static iterator endIterator(BodyType *pBody_)
		{
			return iterator();
		}
	private:
		void increment()
		{
			if (pBody == NULL) {
				return;
			}

			BodyType &body = *pBody;

			++it;
			if (it == body[curLength].end()) {
				++curLength;
				while (curLength < body.size()) {
					BodySliceType &bodyCurLength = body[curLength];
					if (! bodyCurLength.empty()) {
						it = bodyCurLength.begin();
						return;
					}
					++curLength;
				}
				assert(curLength == body.size());
				pBody = NULL;
			}
		}
		bool equal(const iterator &right) const
		{
			if (pBody == NULL) {
				return right.pBody == NULL;
			}
			
			return pBody == right.pBody && curLength == right.curLength && it == right.it;
		}
		typename std::pair<const std::string, DataType> &dereference() const
		{
			return *it;
		}
	};

	class const_iterator : public boost::iterator_facade<const_iterator, value_type, boost::forward_traversal_tag, 
		const typename std::pair<const std::string, DataType>  &> {
	private:
		friend class boost::iterator_core_access;
	public:
		const BodyType *pBody;
		size_t curLength;
		typename BodySliceType::const_iterator it;
	public:
		const_iterator(const iterator &right)
			: pBody(right.pBody), curLength(right.curLength), it(right.it)
		{
		}
		const_iterator(const const_iterator &right)
			: pBody(right.pBody), curLength(right.curLength), it(right.it)
		{
		}
		const_iterator()
			: pBody(NULL), curLength(0), it()
		{
		}
		const_iterator(const BodyType *pBody_, size_t curLength_, typename BodySliceType::const_iterator it_)
			: pBody(pBody_), curLength(curLength_), it(it_)
		{
		}
		static const_iterator beginIterator(const BodyType *pBody_)
		{
			const_iterator i;
			i.pBody = pBody_;
			const BodyType &body = *i.pBody;
			while (i.curLength < body.size()) {
				const BodySliceType &bodyCurLength = body[i.curLength];
				if (! bodyCurLength.empty()) {
					i.it = bodyCurLength.begin();
					return i;
				}
				++i.curLength;
			}
			assert(i.curLength == body.size());
			i.pBody = NULL; // mark of end
			return i;
		}
		static const_iterator endIterator(const BodyType *pBody_)
		{
			return const_iterator();
		}
	private:
		void increment()
		{
			if (pBody == NULL) {
				return;
			}

			const BodyType &body = *pBody;

			++it;
			if (it == body[curLength].end()) {
				++curLength;
				while (curLength < body.size()) {
					const BodySliceType &bodyCurLength = body[curLength];
					if (! bodyCurLength.empty()) {
						it = bodyCurLength.begin();
						return;
					}
					++curLength;
				}
				assert(curLength == body.size());
				pBody = NULL;
			}
		}
		template<typename Iterator>
		bool equal(const Iterator &right) const
		{
			if (pBody == NULL) {
				return right.pBody == NULL;
			}
			
			return pBody == right.pBody && curLength == right.curLength && it == right.it;
		}
		const typename std::pair<const std::string, DataType> &dereference() const
		{
			return *it;
		}
	};
public:
	DataType &operator[](const std::string &key)
	{
		size_t keyLength = key.length();
		if (! (keyLength < body.size())) {
			body.resize(keyLength + 1);
		}
		BodySliceType &bodyCurLength = body[keyLength];
		typename BodySliceType::iterator i = bodyCurLength.find(key);
		if (i != bodyCurLength.end()) {
			return i->second;
		}
		else {
			++countOfData;
			return bodyCurLength[key];
		}
	}

	inline size_type size() const
	{
		return countOfData;
	}

	inline bool empty() const
	{
		return size == 0;
	}

	void clear()
	{
		body.clear();
		countOfData = 0;
	}

	iterator find(const key_type &key)
	{
		size_t keyLength = key.length();
		if (keyLength < body.size()) {
			BodySliceType &bodyCurLength = body[keyLength];
			typename BodySliceType::iterator it = bodyCurLength.find(key);
			if (it != bodyCurLength.end()) {
				return iterator(&body, keyLength, it);
			}
		}
		return iterator::endIterator(&body);
	}

	inline iterator begin()
	{
		return iterator::beginIterator(&body);
	}

	inline iterator end()
	{
		return iterator::endIterator(&body);
	}

	const_iterator find(const key_type &key) const
	{
		size_t keyLength = key.length();
		if (keyLength < body.size()) {
			const BodySliceType &bodyCurLength = body[keyLength];
			typename BodySliceType::const_iterator it = bodyCurLength.find(key);
			if (it != bodyCurLength.end()) {
				return const_iterator(&body, keyLength, it);
			}
		}
		return const_iterator::endIterator(&body);
	}

	inline const_iterator begin() const
	{
		return const_iterator::beginIterator(&body);
	}

	inline const_iterator end() const
	{
		return const_iterator::endIterator(&body);
	}
public:
	void reserve_length(size_t length)
	{
		if (! (length < body.size())) {
			body.reserve(length);
		}
	}
};

#endif // NO_HASH_THINGS

#endif // specialstringmap_h
