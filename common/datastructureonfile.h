#if ! defined DATASTRUCTUREONFILE_H
#define DATASTRUCTUREONFILE_H

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

#include <boost/format.hpp>
#include <boost/cstdint.hpp>

#include "unportable.h"
#include "ffuncrenamer.h"

namespace onfile {

class DynamicBitSet {
private:
	std:: string filePath;
	FILE *pFile;
	unsigned long long bitCount;
public:
	virtual ~DynamicBitSet()
	{
		close();
	}
	DynamicBitSet()
		: pFile(NULL), bitCount(0)
	{
	}
	DynamicBitSet &operator=(const DynamicBitSet &right)
	{
		assert(false);
	}
public:
	bool create(const std:: string &filePath_, bool canBeTemporaryFile)
	{
		std::string flag = "w+b";
#if defined _MSC_VER
		if (canBeTemporaryFile) {
			flag += F_TEMPORARY_FILE_OPTIMIZATION;
		}
#endif

		pFile = ::fopen(filePath_.c_str(), flag.c_str());
		if (pFile == NULL) {
			return false;
		}
		filePath = filePath_;
		bitCount = 0;

		return true;
	}
	void close()
	{
		if (pFile != NULL) {
			fclose(pFile);
			pFile = NULL;
			filePath.clear();
		}
	}
	std:: string getFilePath() const
	{
		return filePath;
	}
	unsigned long long size() const
	{
		return bitCount;
	}
	bool empty() const
	{
		return bitCount == 0;
	}
	void resize(unsigned long long newSize)
	{
		assert(pFile != NULL);
		if (pFile != NULL) {
			unsigned long long newByteCount = (newSize + 8 - 1) / 8;
			if (newByteCount >= LONG_MAX) {
				assert(("too large size for onfile::DynamicBitSet", false));
			}
			if (newSize > bitCount) {
				unsigned long long byteCount = (bitCount + 8 - 1) / 8;
				FSEEK64(pFile, byteCount, SEEK_SET);
				for (unsigned long long i = byteCount; i < newByteCount; ++i) {
					FPUTC(0, pFile);
				}
			}
			bitCount = newSize;
		}
	}
	bool test(unsigned long long index) const
	{
		assert(pFile != NULL);
		assert(index < bitCount);
		if (pFile != NULL) {
			unsigned long long byteIndex = index / 8;
			FSEEK64(pFile, byteIndex, SEEK_SET);
			int bitIndexInTheByte = index % 8;
			unsigned char b;
			FREAD(&b, sizeof(unsigned char), 1, pFile);
			unsigned char mask = 1 << bitIndexInTheByte;
			return (b & mask) != 0;
		}
		return false;
	}
	DynamicBitSet &set(unsigned long long index, bool value = true)
	{
		assert(pFile != NULL);
		assert(index < bitCount);
		if (pFile != NULL) {
			unsigned long long byteIndex = index / 8;
			FSEEK64(pFile, byteIndex, SEEK_SET);
			int bitIndexInTheByte = index % 8;
			unsigned char b;
			FREAD(&b, sizeof(unsigned char), 1, pFile);
			unsigned char mask = 1 << bitIndexInTheByte;
			if (value) {
				b |= mask;
			}
			else {
				b &= ~mask;
			}
			FSEEK64(pFile, byteIndex, SEEK_SET);
			FPUTC(b, pFile);
		}
		return *this;
	}
	DynamicBitSet &setRange(unsigned long long begin, unsigned long long end, bool value = true)
	{
		assert(pFile != NULL);
		assert(begin <= end);
		assert(end < bitCount);
		if (begin != end && pFile != NULL) {
			unsigned long long beginNearestBoundary = (begin + 8 - 1) / 8 * 8;
			unsigned long long index = begin;
			for (; index < beginNearestBoundary && index < end; ++index) {
				this->set(index, value);
			}
			if (index == end) return *this;

			assert(index == beginNearestBoundary);
			unsigned long long endNearestBoundary = end / 8 * 8;
			if (endNearestBoundary > beginNearestBoundary) {
				FSEEK64(pFile, beginNearestBoundary / 8, SEEK_SET);
				unsigned char b = value ? 0xff : 0x00;
				for (unsigned long long i = beginNearestBoundary / 8; i < endNearestBoundary / 8; ++i) {
					FWRITE(&b, sizeof(unsigned char), 1, pFile);
				}
			}
			index = endNearestBoundary;
			if (index == end) return *this;

			for (; index < end; ++index) {
				this->set(index, value);
			}
		}
		return *this;
	}
};

template<typename ItemType>
class Sorter {
private:
	size_t blockSize;
	std:: vector<std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> > blocks;
	std:: string errorMessage;
	std:: string tempInput;
	std:: string tempOutput;
	bool isStableSort;
public:
	virtual ~Sorter()
	{
	}
	Sorter()
		: blockSize(100), tempInput("onfilesorter1.tmp"), tempOutput("onfilesorter2.tmp"), isStableSort(false)
	{
	}
	void setBlockSize(size_t size)
	{
		assert(size > 0);
		blockSize = size;
	}
	void setTempFileNames(const std:: string &temp1, const std:: string &temp2)
	{
		tempInput = temp1;
		tempOutput = temp2;
	}
	void removeTempFiles()
	{
		remove(tempInput.c_str());
		remove(tempOutput.c_str());
	}
	std:: string getErrorMessage() const
	{
		return errorMessage;
	}
	template<typename ItemTypeLess>
	bool sort(const std:: string &sorted, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			ItemTypeLess &itemComparator)
	{
		isStableSort = false;
		return sort_i2(sorted, unsorted, beginPos, itemCount, itemComparator);
	}
	bool sort(const std:: string &sorted, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount)
	{
		isStableSort = false;
		return sort_i2(sorted, unsorted, beginPos, itemCount, std:: less<ItemType>());
	}

	template<typename ItemTypeLess>
	bool stableSort(const std:: string &sorted, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			ItemTypeLess &itemComparator)
	{
		isStableSort = true;
		return sort_i2(sorted, unsorted, beginPos, itemCount, itemComparator);
	}
	bool stableSort(const std:: string &sorted, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount)
	{
		isStableSort = true;
		std::less<ItemType> comparator;
		return sort_i2(sorted, unsorted, beginPos, itemCount, comparator);
	}

	template<typename ItemTypeUniqFunc>
	bool uniq(const std:: string &uniqed, const std:: string &sortedInput, boost::int64_t beginPos, unsigned long long itemCount,
			unsigned long long *pOutputCount, ItemTypeUniqFunc &itemUniq_)
	{
		return uniq_i(uniqed, sortedInput, beginPos, itemCount, pOutputCount, itemUniq_);
	}
	static bool itemUniq(ItemType *pLeft, ItemType *pRight)
	{
		return ! (*pLeft == *pRight);
	}
	bool uniq(const std:: string &uniqed, const std:: string &sortedInput, boost::int64_t beginPos, unsigned long long itemCount,
		unsigned long long *pOutputCount)
	{
		return uniq_i(uniqed, sortedInput, beginPos, itemCount, pOutputCount, itemUniq);
	}

	template<typename ItemTypeLess, typename ItemTypeUniqFunc>
	bool sortAndUniq(const std:: string &uniqed, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			ItemTypeLess &itemComparator,
			unsigned long long *pOutputCount, ItemTypeUniqFunc &itemUniq_)
	{
		if (! sort_i<ItemTypeLess>(unsorted, beginPos, itemCount, itemComparator)) {
			return false;
		}

		// here, tempOutput have the sorted values

		if (! uniq_i(uniqed, tempOutput, beginPos, itemCount, pOutputCount, itemUniq_)) {
			::remove(tempOutput.c_str());
			return false;
		}

		return true;
	}
	bool sortAndUniq(const std:: string &uniqed, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			unsigned long long *pOutputCount)
	{
		if (! sort_i(unsorted, beginPos, itemCount, std:: less<ItemType>())) {
			return false;
		}

		// here, tempOutput have the sorted values

		if (! uniq_i(uniqed, tempOutput, beginPos, itemCount, pOutputCount, itemUniq)) {
			::remove(tempOutput.c_str());
			return false;
		}

		return true;
	}
public:
	virtual size_t freadItem(ItemType *pBuffer, size_t count, FILE *pInput) {
		return FREAD(pBuffer, sizeof(ItemType), count, pInput);
	}
	virtual size_t fwriteItem(const ItemType *pBuffer, size_t count, FILE *pOutput) {
		return FWRITE(pBuffer, sizeof(ItemType), count, pOutput);
	}
private:
	bool copyFile(const std:: string &output, const std:: string &input)
	{
		FILE *pOutput = fopen(output.c_str(), "wb");
		if (pOutput == NULL) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		
		FILE *pInput = fopen(input.c_str(), "rb");
		if (pInput == NULL) {
			fclose(pOutput);
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}

		{
			while (true) {
				int ch = FGETC(pInput);
				if (ch == -1) {
					break;
				}
				FPUTC(ch, pOutput);
			}
		}

		fclose(pInput);
		fclose(pOutput);

		return true;
	}
	template<typename ItemTypeLess>
	bool copySortBlocks(const std:: string &output, const std:: string &input, boost::int64_t beginPos, unsigned long long itemCount,
		const ItemTypeLess itemComparator)
	{
		FILE *pOutput = fopen(output.c_str(), "wb");
		if (pOutput == NULL) {
			errorMessage = (boost::format("can't create a file '%s'") % output).str();
			return false;
		}
		
		FILE *pInput = fopen(input.c_str(), "rb");
		if (pInput == NULL) {
			fclose(pOutput);
			errorMessage = (boost::format("can't open a file '%s'") % input).str();
			return false;
		}

		// copy bytes from the begin of file to the beginPos
		{
			while (true) {
				boost::int64_t pos = FTELL64(pInput);
				if (pos == beginPos) {
					break; // while
				}
				int ch = FGETC(pInput);
				assert(ch != -1);
				FPUTC(ch, pOutput);
			}
		}
		
		std:: vector<ItemType> buffer;
		buffer.resize(blockSize);
		
		unsigned long long remainCount = itemCount;
		while (remainCount > 0) {
			boost::int64_t pos = FTELL64(pInput);

			size_t itemToBeReadCount = buffer.size();
			if (itemToBeReadCount > remainCount) {
				itemToBeReadCount = remainCount;
			}
			size_t readCount = freadItem(&buffer[0], itemToBeReadCount, pInput);
			if (readCount != itemToBeReadCount) {
				errorMessage = "broken file";
				
				fclose(pInput);
				fclose(pOutput);

				return false;
			}
			assert(readCount == itemToBeReadCount);
			
			remainCount -= itemToBeReadCount;
			
			std:: pair<boost::int64_t, unsigned long long> block;
			block.first = pos;
			block.second = readCount;
			blocks.push_back(block);
			
			if (isStableSort) {
				std:: stable_sort(buffer.begin(), buffer.begin() + readCount, itemComparator);
			}
			else {
				std:: sort(buffer.begin(), buffer.begin() + readCount, itemComparator);
			}
			
			fwriteItem(&buffer[0], readCount, pOutput);
			
			if (readCount < buffer.size()) {
				break; // while
			}
		}

		// copy bytes from the endPos to the end of the file
		{
			while (true) {
				int ch = FGETC(pInput);
				if (ch == -1) {
					break;
				}
				FPUTC(ch, pOutput);
			}
		}

		fclose(pInput);
		fclose(pOutput);

		return true;
	}
	struct FRD {
	public:
		FILE *pFile;
		unsigned long long rest;
		ItemType curData;
	public:
		FRD(FILE *pFile_, unsigned long long rest_)
			: pFile(pFile_), rest(rest_), curData()
		{
		}
		FRD(const FRD &right)
			: pFile(right.pFile), rest(right.rest), curData(right.curData)
		{
		}
		FRD()
			: pFile(NULL), rest(0), curData()
		{
		}
		void swap(FRD &right)
		{
			std:: swap(pFile, right.pFile);
			std:: swap(rest, right.rest);
			std:: swap(curData, right.curData);
		}
	};
	template<typename ItemTypeLess>
	bool mergeBlocks(const std:: string &output, const std:: string &input, boost::int64_t beginPos, unsigned long long itemCount,
		const ItemTypeLess itemComparator)
	{
		FILE *pOutput = fopen(output.c_str(), "w+b");
		if (pOutput == NULL) {
			return false;
		}
		FSEEK64(pOutput, beginPos, SEEK_SET);

		std:: vector<std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> > newBlocks;

		size_t bi = 0;
		while (bi < blocks.size()) {
			std:: pair<boost::int64_t, unsigned long long> newBlock;
			newBlock.first = blocks[bi].first;
			newBlock.second = 0;

			std:: vector<FRD> inputs;
			size_t count;
			//size_t blockCount = blocks.size();
			for (count = 0; count < 8 && bi + count < blocks.size(); ++count) {
				FILE *p = fopen(input.c_str(), "rb");
				std:: pair<boost::int64_t/* begin */, unsigned long long/* length */> &b = blocks[bi + count];
				newBlock.second += b.second;
				FSEEK64(p, b.first, SEEK_SET);
				FRD frd(p, b.second);
				if (frd.rest != 0) {
					size_t c = freadItem(&frd.curData, 1, frd.pFile);
					assert(c == 1);
				}
				inputs.push_back(frd);
			}
			
			while (true) {
				for (size_t i = 0; i < inputs.size(); ++i) {
					if (inputs[i].rest == 0) {
						fclose(inputs[i].pFile);
						if (isStableSort) {
							inputs.erase(inputs.begin() + i);
						}
						else {
							inputs[i].swap(inputs.back());
							inputs.pop_back();
						}
					}
				}
				if (inputs.empty()) {
					break; // while true
				}

				size_t minIndex = 0;
				for (size_t i = 1; i < inputs.size(); ++i) {
					if (itemComparator(inputs[i].curData, inputs[minIndex].curData)) {
						minIndex = i;
					}
				}
				
				FRD &frd = inputs[minIndex];
				size_t c = fwriteItem(&frd.curData, 1, pOutput);
				assert(c == 1);
				--frd.rest;
				if (frd.rest != 0) {
					size_t c = freadItem(&frd.curData, 1, frd.pFile);
					assert(c == 1);
				}
			}

			newBlocks.push_back(newBlock);
			bi += count;
		}

		blocks.swap(newBlocks);
		
		fclose(pOutput);
		return true;
	}
private:
	template<typename ItemTypeLess>
	bool sort_i2(const std:: string &sorted, const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			ItemTypeLess &itemComparator)
	{
		if (! sort_i<ItemTypeLess>(unsorted, beginPos, itemCount, itemComparator)) {
			return false;
		}

		// here, tempOutput have the sorted values

		::remove(sorted.c_str());
		int r = ::rename(tempOutput.c_str(), sorted.c_str());
		if (r != 0) {
			errorMessage = (boost::format("can't create a file '%s'") % sorted).str();
			return false;
		}

		return true;
	}
	/*
	sort_i: sort the 'unsorted' file and output the result to tempOutput
	*/
	template<typename ItemTypeLess>
	bool sort_i(const std:: string &unsorted, boost::int64_t beginPos, unsigned long long itemCount,
			ItemTypeLess &itemComparator)
	{
		errorMessage.clear();
		blocks.clear();

		if (! copySortBlocks(tempOutput, unsorted, beginPos, itemCount, itemComparator)) {
			return false;
		}

		if (! copyFile(tempInput, unsorted)) {
			return false;
		}

		while (blocks.size() > 1) {
			tempOutput.swap(tempInput);
			if (! mergeBlocks(tempOutput, tempInput, beginPos, itemCount, itemComparator)) {
				return false;
			}
		}

		::remove(tempInput.c_str());

		return true;
	}
	template<typename ItemTypeUniqFunc>
	bool uniq_i(const std:: string &uniqed, const std:: string &sortedInput, boost::int64_t beginPos, unsigned long long itemCount,
			unsigned long long *pOutputCount, ItemTypeUniqFunc &itemUniq_)
	{
		errorMessage.clear();
		
		FILE *pOutput = fopen(uniqed.c_str(), "wb");
		if (pOutput == NULL) {
			errorMessage = (boost::format("can't create a file '%s'") % uniqed).str();
			return false;
		}
		
		FILE *pInput = fopen(sortedInput.c_str(), "rb");
		if (pInput == NULL) {
			fclose(pOutput);
			errorMessage = (boost::format("can't open a file '%s'") % sortedInput).str();
			return false;
		}

		// copy bytes from the begin of file to the beginPos
		{
			while (true) {
				boost::int64_t pos = FTELL64(pInput);
				if (pos == beginPos) {
					break; // while
				}
				int ch = FGETC(pInput);
				assert(ch != -1);
				FPUTC(ch, pOutput);
			}
		}

		if (itemCount <= 1) {
			if (itemCount == 1) {
				ItemType buffer;
				size_t readCount = freadItem(&buffer, 1, pInput);
				if (readCount != 1) {
					errorMessage = "broken file";

					fclose(pInput);
					fclose(pOutput);

					return false;
				}
			}
		}
		else {
			unsigned long long writeCount = 0;
			unsigned long long remainCount = itemCount;
			ItemType buffer[2];
			size_t readCount = freadItem(&buffer[0], 1, pInput);
			if (readCount != 1) {
				errorMessage = "broken file";

				fclose(pInput);
				fclose(pOutput);

				return false;
			}
			--remainCount;

			ItemType *pLeft = &buffer[0];
			ItemType *pRight = &buffer[1];
			while (remainCount > 0) {
				size_t readCount = freadItem(pRight, 1, pInput);
				if (readCount != 1) {
					errorMessage = "broken file";

					fclose(pInput);
					fclose(pOutput);

					return false;
				}
				--remainCount;
				
				if (itemUniq_(pLeft, pRight)) {
					// the two values are not merged
					fwriteItem(pLeft, 1, pOutput);
					++writeCount;
					std:: swap(pLeft, pRight); // swap the references, not the values
				}
				else {
					// the two values are merged, and the left holds the merged value
				}
			}

			// here, left is holding a value
			fwriteItem(pLeft, 1, pOutput);
			++writeCount;
			
			if (pOutputCount != NULL) {
				*pOutputCount = writeCount;
			}
		}

		// copy bytes from the endPos to the end of the file
		{
			while (true) {
				int ch = FGETC(pInput);
				if (ch == -1) {
					break;
				}
				FPUTC(ch, pOutput);
			}
		}

		fclose(pInput);
		fclose(pOutput);

		return true;
	}
};

template<typename ItemType>
class Array {
private:
	std:: string filePath;
	FILE *pFile;
	unsigned long long itemCount;
	ItemType fillingValue;
public:
	virtual ~Array()
	{
		close();
	}
	Array()
		: pFile(NULL), itemCount(0)
	{
	}
	Array &operator=(const Array<ItemType> &right)
	{
		assert(false);
	}
public:
	bool create(const std:: string &filePath_, bool canBeTemporaryFile)
	{
		std::string flag = "w+b";
#if defined _MSC_VER
		if (canBeTemporaryFile) {
			flag += F_TEMPORARY_FILE_OPTIMIZATION;
		}
#endif

		pFile = fopen(filePath_.c_str(), flag.c_str());
		if (pFile == NULL) {
			return false;
		}
		filePath = filePath_;
		itemCount = 0;

		return true;
	}
	void close()
	{
		if (pFile != NULL) {
			fclose(pFile);
			pFile = NULL;
			filePath.clear();
		}
	}
	std:: string getFilePath() const
	{
		return filePath;
	}
	unsigned long long size() const
	{
		return itemCount;
	}
	bool empty() const
	{
		return itemCount == 0;
	}
	void append(const ItemType &value)
	{
		assert(pFile != NULL);
		if (pFile != NULL) {
			unsigned long long newByteCount = (itemCount + 1) * sizeof(ItemType);
			if (newByteCount >= LONG_MAX) {
				assert(("too large size for onfile::Array<ItemType>", false));
			}
			unsigned long long byteCount = itemCount * sizeof(ItemType);
			FSEEK64(pFile, byteCount, SEEK_SET);
			FWRITE(&value,  sizeof(ItemType), 1, pFile);
			++itemCount;
		}
	}
	void extend(ItemType *aItems, unsigned long long count)
	{
		assert(pFile != NULL);
		assert(count >= 0);
		if (pFile != NULL) {
			unsigned long long newByteCount = (itemCount + count) * sizeof(ItemType);
			if (newByteCount >= LONG_MAX) {
				assert(("too large size for onfile::Array<ItemType>", false));
			}
			unsigned long long byteCount = itemCount * sizeof(ItemType);
			FSEEK64(pFile, byteCount, SEEK_SET);
			FWRITE(aItems, sizeof(ItemType), count, pFile);
			itemCount += count;
		}
	}
	void resize(unsigned long long newSize)
	{
		assert(pFile != NULL);
		if (pFile != NULL) {
			unsigned long long newByteCount = newSize * sizeof(ItemType);
			if (newByteCount >= LONG_MAX) {
				assert(("too large size for onfile::Array<ItemType>", false));
			}
			if (newSize > itemCount) {
				unsigned long long byteCount = itemCount * sizeof(ItemType);
				FSEEK64(pFile, byteCount, SEEK_SET);
				for (unsigned long long i = itemCount; i < newSize; ++i) {
					FWRITE(&fillingValue, sizeof(ItemType), 1, pFile);
				}
			}
			itemCount = newSize;
		}
	}
	void get(ItemType *pValue, unsigned long long index) const
	{
		assert(pFile != NULL);
		assert(index < itemCount);
		if (pFile != NULL) {
			unsigned long long byteIndex = index * sizeof(ItemType);
			FSEEK64(pFile, byteIndex, SEEK_SET);
			FREAD(pValue, sizeof(ItemType), 1, pFile);
		}
		else {
			assert(false);
		}
	}
	Array<ItemType> &set(unsigned long long index, const ItemType &value)
	{
		assert(pFile != NULL);
		assert(index < itemCount);
		if (pFile != NULL) {
			unsigned long long byteIndex = index * sizeof(ItemType);
			FSEEK64(pFile, byteIndex, SEEK_SET);
			FWRITE(&value, sizeof(ItemType), 1, pFile);
		}
		else {
			assert(false);
		}
		return *this;
	}
};


}; // namespace

#endif // DATASTRUCTUREONFILE_H

