#if ! defined FILTERINGMAIN_H
#define FILTERINGMAIN_H

#include <map>
#include <vector>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <limits>

#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/array.hpp>

#include "../common/datastructureonfile.h"
#include "../common/filestructwrapper.h"
#include "rawclonepairdata.h"
#include "ccfxcommon.h"

template<typename IntType>
struct Range {
public:
	typedef std::pair<IntType, IntType> Type;
};

template<typename IntType>
struct RangeVector {
public:
	typedef std::vector<typename Range<IntType>::Type > Type;
};

template<typename DstIntType, typename SrcIntType> 
bool convert_ranges(typename RangeVector<DstIntType>::Type *pDstRanges, const typename RangeVector<SrcIntType>::Type &srcRanges, 
		DstIntType allowedMinValue, DstIntType allowedMaxValue)
{
	assert(pDstRanges != NULL);
	typename RangeVector<DstIntType>::Type &r = *pDstRanges;
	r.clear();
	r.reserve(srcRanges.size());

	for (typename RangeVector<SrcIntType>::Type::const_iterator p = srcRanges.begin(); p != srcRanges.end(); ++p) {
		const typename Range<SrcIntType>::Type &src = *p;
		if (! (allowedMinValue <= src.first && src.first <= allowedMaxValue)) {
			return false;
		}
		if (! (allowedMinValue <= src.second && src.second <= allowedMaxValue)) {
			return false;
		}
		typename Range<DstIntType>::Type dst;
		dst.first = src.first;
		dst.second = src.second;
		r.push_back(dst);
	}
	return true;
}

class FilteringMain {
private:
	friend class Filter;
	class Filter : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		FilteringMain &base;
	public:		
		Filter(FilteringMain *pBase)
			: base(*pBase)
		{
		}
		bool isValidFileID(int fileID)
		{
			if (! base.fileRangeMap.empty()) {
				if (fileID > base.maxFileID) {
					return false;
				}
				return base.fileRangeMap[fileID];
			}
			else if (! base.notFileRangeMap.empty()) {
				if (fileID > base.maxFileID) {
					return true;
				}
				return ! base.notFileRangeMap[fileID];
			}
			else {
				return true; // will not do filtering by fileID
			}
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			if (! base.cloneRangeMap.empty()) {
				if (cloneID >= base.cloneRangeMap.size()) {
					return false;
				}
				return base.cloneRangeMap.test(cloneID);
			}
			else if (! base.notCloneRangeMap.empty()) {
				if (cloneID >= base.notCloneRangeMap.size()) {
					return true;
				}
				return ! base.notCloneRangeMap.test(cloneID);
			}
			else {
				return true; // will not do filtering by cloneID
			}
		}
		virtual bool isValidClonePair(const rawclonepair::RawFileBeginEnd &left, const rawclonepair::RawFileBeginEnd &right, boost::uint64_t cloneID)
		{
			if (! (isValidCloneID(cloneID) && isValidFileID(left.file) && isValidFileID(right.file))) {
				return false;
			}

			if (! base.innerCloneValid) {
				return left.file != right.file;
			}
			else if (! base.interCloneValid) {
				return left.file == right.file;
			}
			else if (! base.groupIndex.empty()) {
				int lg = left.file >= base.groupIndex.size() ? 0 : base.groupIndex[left.file];
				if (lg == 0) {
					return false;
				}
				int rg = right.file >= base.groupIndex.size() ? 0 : base.groupIndex[right.file];
				return lg == rg;
			}
			else if (! base.notGroupIndex.empty()) {
				int lg = left.file >= base.notGroupIndex.size() ? 0 : base.notGroupIndex[left.file];
				if (lg == 0) {
					return true;
				}
				int rg = right.file >= base.notGroupIndex.size() ? 0 : base.notGroupIndex[right.file];
				return lg != rg;
			}
			else {
				return true;
			}
		}
		virtual void transformFiles(std:: vector<rawclonepair::RawClonePairFileTransformer::RawFileData> *pFiles)
		{
			if (! base.fileOrder.empty()) {
				std:: map<int, size_t> id2index;
				for (size_t i = 0; i < (*pFiles).size(); ++i) {
					const rawclonepair::RawClonePairFileTransformer::RawFileData &file = (*pFiles)[i];
					id2index[file.fileID] = i;
				}
				std:: vector<rawclonepair::RawClonePairFileTransformer::RawFileData> filtered;
				filtered.reserve((*pFiles).size());
				for (size_t i = 0; i < base.fileOrder.size(); ++i) {
					int fileID = base.fileOrder[i];
					if (isValidFileID(fileID)) {
						std:: map<int, size_t>::const_iterator j = id2index.find(fileID);
						if (j != id2index.end()) {
							size_t index = j->second;
							const rawclonepair::RawClonePairFileTransformer::RawFileData &file = (*pFiles)[index];
							filtered.push_back(file);
						}
					}
				}
				(*pFiles).swap(filtered);
			}
			else {
				rawclonepair::RawClonePairFileTransformer::FilterFileByFile::transformFiles(pFiles);
			}
		}
	};

private:
	std:: vector<Range<int>::Type > fileRange;
	std:: vector<Range<int>::Type > notFileRange;
	std:: vector<Range<long long>::Type > cloneRange;
	std:: vector<Range<long long>::Type > notCloneRange;
	std:: vector<int> fileOrder;
	std:: string fileSelectionList;
	std:: string notFileSelectionList;
	std:: string cloneSelectionList;
	std:: string notCloneSelectionList;
	std:: string fileOrderList;
	bool innerCloneValid;
	bool interCloneValid;
	std:: vector<int> groupIndex;
	std:: vector<int> notGroupIndex;
	int group_number;
	bool optionBinarySelectionList;

	int maxFileID;
	boost::dynamic_bitset<> fileRangeMap;
	boost::dynamic_bitset<> notFileRangeMap;
	onfile::DynamicBitSet cloneRangeMap;
	onfile::DynamicBitSet notCloneRangeMap;

	enum analyze_state_t {
		none,
		in_file_range,
		in_not_file_range,
		in_file_order,
		in_clone_range,
		in_not_clone_range,
		in_group_range,
		in_not_group_range
	};
	analyze_state_t state;
	std:: string errorMessage;

private:
	bool scan_range(const std:: string &str, std:: vector<std:: pair<long long, long long> > *pRanges)
	{
		size_t i = 0;
		while (i < str.length()) {
			int num1Begin = i;
			while (i < str.length()) {
				if ('0' <= str[i] && str[i] <= '9') {
					++i;
				}
				else {
					break; // while
				}
			}
			int num1End = i;
			std:: string s = str.substr(num1Begin, num1End - num1Begin);
			long long num1;
			try {
				num1 = boost::lexical_cast<long long>(s);
			}
			catch (boost::bad_lexical_cast &) {
				return false;
			}
			if (i < str.length() && str[i] == '-') {
				++i;
				int num2Begin = i;
				while (i < str.length()) {
					if ('0' <= str[i] && str[i] <= '9') {
						++i;
					}
					else {
						break; // while
					}
				}
				int num2End = i;
				std:: string s = str.substr(num2Begin, num2End - num2Begin);
				long long num2;
				try {
					num2 = boost::lexical_cast<long long>(s);
				}
				catch (boost::bad_lexical_cast &) {
					return false;
				}
				if (num2 <= num1) {
					errorMessage = "invalid range string";
				}
				(*pRanges).push_back(std:: pair<long long, long long>(num1, num2));
			}
			else {
				(*pRanges).push_back(std:: pair<long long, long long>(num1, num1));
			}
			if (i < str.length()) {
				if (str[i] == ',') {
					++i;
				}
				else {
					errorMessage = "invalid range string";
					return false;
				}
			}
		}
		return true;
	}
	size_t analyze_command(size_t *pI, const std::vector<std::string> &argv)
	{
		// -f 1-3,5,7,9   1,2,3,5,7,9
		
		size_t &ai = *pI;
		std:: string argi = argv[ai];

		if (argi.length() == 0) {
			errorMessage = "invalid string";
			return false;
		}

		if (argi[0] == '-') {
			if (argi == "-f") {
				if (! (notFileRange.empty() && fileSelectionList.empty() && fileOrder.empty() && fileOrderList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				state = in_file_range;
				++ai;
			}
			else if (argi == "-!f") {
				if (! (fileRange.empty() && fileSelectionList.empty() && fileOrder.empty() && fileOrderList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				state = in_not_file_range;
				++ai;
			}
			else if (argi == "-fi" || argi == "-fb") {
				if (! (fileRange.empty() && notFileRange.empty() && notFileSelectionList.empty() && fileOrder.empty() && fileOrderList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				if (! (ai + 1 < argv.size())) {
					errorMessage = (boost::format("command %s requires an argument") % argi).str();
					return false;
				}
				fileSelectionList = argv[ai + 1];
				if (argi == "-fb") {
					optionBinarySelectionList = true;
				}
				state = none;
				ai += 2;
			}
			else if (argi == "-!fi" || argi == "-!fb") {
				if (! (fileRange.empty() && notFileRange.empty() && fileSelectionList.empty() && fileOrder.empty() && fileOrderList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				if (! (ai + 1 < argv.size())) {
					errorMessage = (boost::format("command %s requires an argument") % argi).str();
					return false;
				}
				notFileSelectionList = argv[ai + 1];
				if (argi == "-fb") {
					optionBinarySelectionList = true;
				}
				state = none;
				ai += 2;
			}
			else if (argi == "-g") {
				if (! (fileRange.empty() && notFileRange.empty() && fileSelectionList.empty() && notFileSelectionList.empty() && fileSelectionList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				state = in_file_order;
				++ai;
			}
			else if (argi == "-gi" || argi == "-gb") {
				if (! (fileRange.empty() && notFileRange.empty() && fileSelectionList.empty() && notFileSelectionList.empty() && fileSelectionList.empty())) {
					errorMessage = "commands -f, -!f, -fi, -!fi, -g, and -gi are exclusive";
					return false;
				}
				if (! (ai + 1 < argv.size())) {
					errorMessage = (boost::format("command %s requires an argument") % argi).str();
					return false;
				}
				fileOrderList = argv[ai + 1];
				optionBinarySelectionList = true;
				state = none;
				ai += 2;
			}
			else if (argi == "-c") {
				if (! (notCloneRange.empty() && cloneSelectionList.empty() && notCloneSelectionList.empty())) {
					errorMessage = "commands -c, -!c, and -ci are exclusive";
					return false;
				}
				state = in_clone_range;
				++ai;
			}
			else if (argi == "-!c") {
				if (! (cloneRange.empty() && cloneSelectionList.empty() && notCloneSelectionList.empty())) {
					errorMessage = "commands -c, -!c, and -ci are exclusive";
					return false;
				}
				state = in_not_clone_range;
				++ai;
			}
			else if (argi == "-ci" || argi == "-cb") {
				if (! (cloneRange.empty() && notCloneRange.empty() && notCloneSelectionList.empty())) {
					errorMessage = "commands -c, -!c, and -ci are exclusive";
					return false;
				}
				if (! (ai + 1 < argv.size())) {
					errorMessage = (boost::format("command %s requires an argument") % argi).str();
					return false;
				}
				cloneSelectionList = argv[ai + 1];
				if (argi == "-cb") {
					optionBinarySelectionList = true;
				}
				state = none;
				ai += 2;
			}
			else if (argi == "-!ci" || argi == "-!cb") {
				if (! (cloneRange.empty() && notCloneRange.empty() && cloneSelectionList.empty())) {
					errorMessage = "commands -c, -!c, -ci, and -!ci are exclusive";
					return false;
				}
				if (! (ai + 1 < argv.size())) {
					errorMessage = (boost::format("command %s requires an argument") % argi).str();
					return false;
				}
				notCloneSelectionList = argv[ai + 1];
				if (argi == "-cb") {
					optionBinarySelectionList = true;
				}
				state = none;
				ai += 2;
			}
			else if (argi == "-p0") {
				if (! innerCloneValid) {
					errorMessage = "commands -p0 and -!p0 are exclusive";
					return false;
				}
				if (! groupIndex.empty()) {
					errorMessage = "commands -p0 and -p are exclusive";
					return false;
				}
				if (! notGroupIndex.empty()) {
					errorMessage = "commands -p0 and -!p are exclusive";
					return false;
				}
				interCloneValid = false;
				state = none;
				++ai;
			}
			else if (argi == "-!p0") {
				if (! interCloneValid) {
					errorMessage = "commands -p0 and -!p0 are exclusive";
					return false;
				}
				if (! groupIndex.empty()) {
					errorMessage = "commands -!p0 and -p are exclusive";
					return false;
				}
				if (! notGroupIndex.empty()) {
					errorMessage = "commands -!p0 and -!p are exclusive";
					return false;
				}
				innerCloneValid = false;
				state = none;
				++ai;
			}
			else if (argi == "-p") {
				if (! notGroupIndex.empty()) {
					errorMessage = "commands -p and -!p are exclusive";
					return false;
				}
				if (! interCloneValid) {
					errorMessage = "commands -p0 and -p are exclusive";
					return false;
				}
				if (! innerCloneValid) {
					errorMessage = "commands -!p0 and -p are exclusive";
					return false;
				}
				state = in_group_range;
				++group_number;
				++ai;
			}
			else if (argi == "-!p") {
				if (! groupIndex.empty()) {
					errorMessage = "commands -p and -!p are exclusive";
					return false;
				}
				if (! interCloneValid) {
					errorMessage = "commands -p0 and -!p are exclusive";
					return false;
				}
				if (! innerCloneValid) {
					errorMessage = "commands -!p0 and -!p are exclusive";
					return false;
				}
				state = in_not_group_range;
				++group_number;
				++ai;
			}
			else {
				errorMessage = std:: string("unknown comand: '") + argi + "'";
				return false;
			}
		}
		else {
			std:: vector<Range<long long>::Type > rangesl;
			if (! scan_range(argi, &rangesl)) {
				errorMessage = std::string("invalild range: '") + argi + "'";
				return false;
			}
			switch (state) {
			case in_file_range:
				{
					RangeVector<int>::Type ranges;
					if (! convert_ranges<int, long long>(&ranges, rangesl, 0, std::numeric_limits<int>::max())) {
						errorMessage = std::string("invalild range: '") + argi + "'";
						return false;
					}
					fileRange.insert(fileRange.end(), ranges.begin(), ranges.end());
					for (size_t j = 0; j < ranges.size(); ++j) {
						const std:: pair<int, int> &range = ranges[j];
						if (range.second > maxFileID) {
							maxFileID = range.second;
						}
					}
				}
				++ai;
				break;
			case in_not_file_range:
				{
					RangeVector<int>::Type ranges;
					if (! convert_ranges<int, long long>(&ranges, rangesl, 0, std::numeric_limits<int>::max())) {
						errorMessage = std::string("invalild range: '") + argi + "'";
						return false;
					}
					notFileRange.insert(notFileRange.end(), ranges.begin(), ranges.end());
					for (size_t j = 0; j < ranges.size(); ++j) {
						const std:: pair<int, int> &range = ranges[j];
						if (range.second > maxFileID) {
							maxFileID = range.second;
						}
					}
				}
				++ai;
				break;
			case in_file_order:
				{
					RangeVector<int>::Type ranges;
					if (! convert_ranges<int, long long>(&ranges, rangesl, 0, std::numeric_limits<int>::max())) {
						errorMessage = std::string("invalild range: '") + argi + "'";
						return false;
					}
					for (size_t j = 0; j < ranges.size(); ++j) {
						const Range<int>::Type &range = ranges[j];
						if (range.second > maxFileID) {
							maxFileID = range.second;
						}
						int b = range.first;
						int e = range.second;
						for (int k = b; k <= e; ++k) {
							fileOrder.push_back(k);
						}
					}
				}
				++ai;
				break;
			case in_clone_range:
				{
					cloneRange.insert(cloneRange.end(), rangesl.begin(), rangesl.end());
				}
				++ai;
				break;
			case in_not_clone_range:
				{
					notCloneRange.insert(notCloneRange.end(), rangesl.begin(), rangesl.end());
				}
				++ai;
				break;
			case in_group_range:
				{
					RangeVector<int>::Type ranges;
					if (! convert_ranges<int, long long>(&ranges, rangesl, 0, std::numeric_limits<int>::max())) {
						errorMessage = std::string("invalild range: '") + argi + "'";
						return false;
					}
					for (size_t j = 0; j < ranges.size(); ++j) {
						const std:: pair<int, int> &range = ranges[j];
						if (! (range.second + 1 < groupIndex.size())) {
							groupIndex.resize(range.second + 1, 0);
						}
						for (int fi = range.first; fi <= range.second; ++fi) {
							if (groupIndex[fi] != 0 && groupIndex[fi] != group_number) {
								errorMessage = "no file can belong two groups";
								return false;
							}
							groupIndex[fi] = group_number;
						}
					}
				}
				++ai;
				break;
			case in_not_group_range:
				{
					RangeVector<int>::Type ranges;
					if (! convert_ranges<int, long long>(&ranges, rangesl, 0, std::numeric_limits<int>::max())) {
						errorMessage = std::string("invalild range: '") + argi + "'";
						return false;
					}
					for (size_t j = 0; j < ranges.size(); ++j) {
						const Range<int>::Type &range = ranges[j];
						if (! (range.second + 1 < groupIndex.size())) {
							notGroupIndex.resize(range.second + 1, 0);
						}
						for (int fi = range.first; fi <= range.second; ++fi) {
							if (notGroupIndex[fi] != 0 && notGroupIndex[fi] != group_number) {
								errorMessage = "no file can belong two groups";
								return false;
							}
							notGroupIndex[fi] = group_number;
						}
					}
				}
				++ai;
				break;
			case none:
				errorMessage = "invalid string";
				return false;
			}
		}
		return true;
	}
	template<typename DynamicBitSetType>
	static bool getList(DynamicBitSetType *pBits, const std:: string &listFile)
	{
		std:: ifstream is;
		is.open(listFile.c_str(), std:: ios::in | std:: ios::binary);
		if (! is.is_open()) {
			return false;
		}
		
		std:: string str;
		while (! is.eof()) {
			std:: getline(is, str, '\n');
			if (str.empty() && is.eof()) {
				break; // while
			}
			if (str.length() >= 1 && str[str.length() - 1] == '\r') {
				str.resize(str.length() - 1);
			}
			long long num;
			try {
				num = boost::lexical_cast<long long>(str);
			}
			catch (boost::bad_lexical_cast &) {
				return false;
			}
			assert(num + 1 < std::numeric_limits<size_t>::max());
			if (! (num < (*pBits).size())) {
				(*pBits).resize(num + 1);
			}
			(*pBits).set(num, true);
		}
		is.close();

		return true;
	}
	template<typename DynamicBitSetType>
	static bool getListBinary(DynamicBitSetType *pBits, const std:: string &listFileBinary)
	{
		FileStructWrapper pf(listFileBinary, "rb");
		if (! pf) {
			return false;
		}
		
		while (true) {
			long long num;
			if (FREAD(&num, sizeof(long long), 1, pf) != 1) {
				break; // while
			}
			assert(num + 1 < std::numeric_limits<size_t>::max());
			if (! (num < (*pBits).size())) {
				(*pBits).resize(num + 1);
			}
			(*pBits).set(num, true);
		}

		return true;
	}
	template<typename DynamicBitSetType>
	static bool getList(DynamicBitSetType *pBits, const std:: string &listFile, long long maxSize)
	{
		std:: ifstream is;
		is.open(listFile.c_str(), std:: ios::in | std:: ios::binary);
		if (! is.is_open()) {
			return false;
		}
		
		std:: string str;
		while (! is.eof()) {
			std:: getline(is, str, '\n');
			if (str.empty() && is.eof()) {
				break; // while
			}
			if (str.length() >= 1 && str[str.length() - 1] == '\r') {
				str.resize(str.length() - 1);
			}
			long long num;
			try {
				num = boost::lexical_cast<long long>(str);
			}
			catch (boost::bad_lexical_cast &e) {
				return false;
			}
			if (! (num < maxSize)) {
				return false;
			}
			if (! (num < (*pBits).size())) {
				(*pBits).resize(num + 1);
			}
			(*pBits).set(num, true);
		}
		is.close();

		return true;
	}
	template<typename DynamicBitSetType>
	static bool getListBinary(DynamicBitSetType *pBits, const std:: string &listFileBinary, long long maxSize)
	{
		FileStructWrapper pf(listFileBinary, "rb");
		if (! pf) {
			return false;
		}
		
		while (true) {
			long long num;
			if (FREAD(&num, sizeof(long long), 1, pf) != 1) {
				break; // while
			}
			if (! (num < maxSize)) {
				return false;
			}
			if (! (num < (*pBits).size())) {
				(*pBits).resize(num + 1);
			}
			(*pBits).set(num, true);
		}

		return true;
	}
	static bool getList(std::vector<int> *pList, int *pMaxValue, const std:: string &listFile)
	{
		std:: ifstream is;
		is.open(listFile.c_str(), std:: ios::in | std:: ios::binary);
		if (! is.is_open()) {
			return false;
		}
		
		bool firstOne = true;
		std:: string str;
		while (! is.eof()) {
			std:: getline(is, str, '\n');
			if (str.empty() && is.eof()) {
				break; // while
			}
			if (str.length() >= 1 && str[str.length() - 1] == '\r') {
				str.resize(str.length() - 1);
			}
			long long num;
			try {
				num = boost::lexical_cast<long long>(str);
			}
			catch (boost::bad_lexical_cast &) {
				return false;
			}
			assert(num < std::numeric_limits<int>::max());
			int value = num;
			(*pList).push_back(value);
			if (firstOne) {
				*pMaxValue = value;
			}
			else {
				if (value > *pMaxValue) {
					*pMaxValue = value;
				}
			}
		}
		is.close();

		return true;
	}
	static bool getListBinary(std::vector<int> *pList, int *pMaxValue, const std:: string &listFileBinary)
	{
		FileStructWrapper pf(listFileBinary, "rb");
		if (! pf) {
			return false;
		}
		
		bool firstOne = true;
		while (true) {
			long long num;
			if (FREAD(&num, sizeof(long long), 1, pf) != 1) {
				break; // while
			}
			assert(num <= std::numeric_limits<int>::max());
			int value = num;
			(*pList).push_back(value);
			if (firstOne) {
				*pMaxValue = value;
			}
			else {
				if (value > *pMaxValue) {
					*pMaxValue = value;
				}
			}
		}

		return true;
	}
public:
	int main(const std::vector<std::string> &argv) 
	{
		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std::cout <<
				"Usage: ccfx S in.ccfxd -o out.ccfxd COMMANDS..." "\n"
				"  Extracts a subset from a clone data." "\n"
				"Command" "\n"
				"  -c 1,3-4: selects clones of the specified clone-set IDs." "\n"
				"  -!c 1,3-4: eliminates the clones" "\n"
				"  -ci cloneidlist: selects clones in the list." "\n"
				"  -!ci cloneidlist: eliminates clones in the list." "\n"
				"  -f 1,3-4: selects files of the specified file IDs." "\n"
				"  -!f 1,3-4: elimnates the files." "\n"
				"  -fi fileidlist: selects files in the list." "\n"
				"  -!fi fileidlist: eliminates files in the list." "\n"
				"  -g 1,3-4: re-order files." "\n"
				"  -gi fileidlist: re-order files by the list." "\n"
				"  -p0: selects clone pairs within a source file." "\n"
				"  -!p0: eliminates clone pairs within a source file." "\n"
				"  -p 1,3-4: selects clone pairs between two source files in a group." "\n"
				"  -!p 1,3-4: eliminates clone pairs between two source files in a group." "\n"
				;
			return 0;
		}

		std:: string inputFile;
		std:: string outputFile;

		innerCloneValid = true;
		interCloneValid = true;
		group_number = 0;
		optionBinarySelectionList = false;

		size_t i = 2;
		if (! (i < argv.size())) {
			std:: cerr << "error: option -o is not given" << std:: endl;
			return 1;
		}
		std:: string argi = argv[i];
		if (boost::starts_with(argi, "-")) {
			inputFile = "a" + pairBinaryDiploidExtension;
		}
		else {
			inputFile = argi;
			boost::array<std::string, 3> exts = {{ pairBinaryDiploidExtension, ".icetd", ".tmp" }};
			force_extension(&inputFile, exts.begin(), exts.end());
			++i;
		}
		if (! (i < argv.size())) {
			std:: cerr << "error: option -o required" << std:: endl;
			return 1;
		}
		argi = argv[i];
		if (argi == "-o") {
			if (! (i + 1 < argv.size())) {
				std:: cerr << "error: option -o requires an argument" << std:: endl;
				return 1;
			}
			outputFile = argv[i + 1];
			boost::array<std::string, 3> exts = {{ pairBinaryDiploidExtension, ".icetd", ".tmp" }};
			force_extension(&outputFile, exts.begin(), exts.end());
			i += 2;
		}
		else {
			std:: cerr << "error: option -o is not given" << std:: endl;
			return 1;
		}

		maxFileID = 0;

		state = none;
		while (i < argv.size()) {
			if (! analyze_command(&i, argv)) {
				std:: cerr << "error: " << errorMessage << std:: endl;
				return 1;
			}
		}

		if (! fileRange.empty()) {
			fileRangeMap.resize(maxFileID + 1);
			for (size_t i = 0; i < fileRange.size(); ++i) {
				int from = fileRange[i].first;
				int to = fileRange[i].second;
				for (int j = from; j <= to; ++j) {
					fileRangeMap[j] = true;
				}
			}
		}
		else if (! notFileRange.empty()) {
			notFileRangeMap.resize(maxFileID + 1);
			for (size_t i = 0; i < notFileRange.size(); ++i) {
				int from = notFileRange[i].first;
				int to = notFileRange[i].second;
				for (int j = from; j <= to; ++j) {
					notFileRangeMap[j] = true;
				}
			}
		}
		else if (! fileSelectionList.empty()) {
			if (optionBinarySelectionList) {
				if (! getListBinary(&fileRangeMap, fileSelectionList)) {
					std:: cerr << "error: can't open file '" << fileSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (! getList(&fileRangeMap, fileSelectionList)) {
					std:: cerr << "error: can't open file '" << fileSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			maxFileID = fileRangeMap.empty() ? 0 : fileRangeMap.size() - 1;
		}
		else if (! notFileSelectionList.empty()) {
			if (optionBinarySelectionList) {
				if (! getListBinary(&notFileRangeMap, notFileSelectionList)) {
					std:: cerr << "error: can't open file '" << notFileSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (! getList(&notFileRangeMap, notFileSelectionList)) {
					std:: cerr << "error: can't open file '" << notFileSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			maxFileID = notFileRangeMap.empty() ? 0 : notFileRangeMap.size() - 1;
		}
		else if (! fileOrder.empty()) {
			fileRangeMap.resize(maxFileID + 1);
			for (size_t i = 0; i < fileOrder.size(); ++i) {
				int fileID = fileOrder[i];
				if (fileRangeMap.test(fileID)) {
					std:: cerr << "error: same file ID appeared twice in file oder" << std::endl;
					return 1;
				}
				fileRangeMap.set(fileID);
			}
		}
		else if (! fileOrderList.empty()) {
			int maxID;
			if (optionBinarySelectionList) {
				if (! getList(&fileOrder, &maxID, fileOrderList)) {
					std:: cerr << "error: can't open file '" << fileOrderList << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (! getListBinary(&fileOrder, &maxID, fileOrderList)) {
					std:: cerr << "error: can't open file '" << fileOrderList << "'" << std:: endl;
					return 1;
				}
			}
			maxFileID = maxID;
			fileRangeMap.resize(maxFileID + 1);
			for (size_t i = 0; i < fileOrder.size(); ++i) {
				int fileID = fileOrder[i];
				if (fileRangeMap.test(fileID)) {
					std:: cerr << "error: same file ID appeared twice in file oder" << std::endl;
					return 1;
				}
				fileRangeMap.set(fileID);
			}
		}
		
		const std:: string tempFile = ::make_temp_file_on_the_same_directory(outputFile, "ccfxfiltering", ".tmp");
		if (! cloneRange.empty()) {
			if (! cloneRangeMap.create(tempFile, true)) {
				std:: cerr << "error: can't create temp file" << std:: endl;
				return 1;
			}
			for (size_t i = 0; i < cloneRange.size(); ++i) {
				cloneRangeMap.resize(cloneRange[i].second + 1);
				long long from = cloneRange[i].first;
				long long to = cloneRange[i].second;
				cloneRangeMap.setRange(from, to + 1, true);
			}
		}
		else if (! notCloneRange.empty()) {
			if (! notCloneRangeMap.create(tempFile, true)) {
				std:: cerr << "error: can't create temp file" << std:: endl;
				return 1;
			}
			for (size_t i = 0; i < notCloneRange.size(); ++i) {
				notCloneRangeMap.resize(notCloneRange[i].second + 1);
				long long from = notCloneRange[i].first;
				long long to = notCloneRange[i].second;
				notCloneRangeMap.setRange(from, to + 1, true);
			}
		}
		else if (! cloneSelectionList.empty()) {
			if (! cloneRangeMap.create(tempFile, true)) {
				std:: cerr << "error: can't create temp file" << std:: endl;
				return 1;
			}
			if (optionBinarySelectionList) {
				if (! getListBinary(&cloneRangeMap, cloneSelectionList)) {
					std:: cerr << "error: can't open file '" << cloneSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (! getList(&cloneRangeMap, cloneSelectionList)) {
					std:: cerr << "error: can't open file '" << cloneSelectionList << "'" << std:: endl;
					return 1;
				}
			}
		}
		else if (! notCloneSelectionList.empty()) {
			if (! notCloneRangeMap.create(tempFile, true)) {
				std:: cerr << "error: can't create temp file" << std:: endl;
				return 1;
			}
			if (optionBinarySelectionList) {
				if (! getListBinary(&notCloneRangeMap, notCloneSelectionList)) {
					std:: cerr << "error: can't open file '" << notCloneSelectionList << "'" << std:: endl;
					return 1;
				}
			}
			else {
				if (! getList(&notCloneRangeMap, notCloneSelectionList)) {
					std:: cerr << "error: can't open file '" << notCloneSelectionList << "'" << std:: endl;
					return 1;
				}
			}
		}

		rawclonepair::RawClonePairFileTransformer sorter;
		Filter filter(this);
		if (! sorter.filterFileByFile(outputFile, inputFile, &filter)) {
			std::cerr << "error: " << sorter.getErrorMessage() << std::endl;
			return 1;
		}

		if (! cloneRangeMap.getFilePath().empty() || ! notCloneRangeMap.getFilePath().empty()) {
			cloneRangeMap.close();
			notCloneRangeMap.close();
			remove(tempFile.c_str());
		}

		return 0;
	}
};


#endif // defined FILTERINGMAIN_H
