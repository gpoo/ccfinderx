#if ! defined METRICMAIN_H
#define METRICMAIN_H

#include <cassert>
#include <cstdio>

#include <stdexcept>
#include <string>
#include <map>
#include "../common/hash_set_includer.h"
#include <set>
#include <iostream>
#include <vector>
#include <algorithm>

#include <boost/dynamic_bitset.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/array.hpp>

#include "../common/ffuncrenamer.h"

#include "../repdet/repdet.h"
#include "ccfxconstants.h"
#include "ccfxcommon.h"
#include "rawclonepairdata.h"
#include "../common/filestructwrapper.h"

#ifdef _MSC_VER
#undef min
#undef max
#endif

namespace metrics {

template <typename value_type>
struct MinMax {
public:
	value_type min;
	value_type max;
public:
	inline MinMax()
		: min(), max()
	{
	}
	inline MinMax(value_type min_, value_type max_)
		: min(min_), max(max_)
	{
	}
	inline MinMax(const MinMax<value_type> &rhs)
		: min(rhs.min), max(rhs.max)
	{
	}
	inline MinMax(value_type initValue)
		: min(initValue), max(initValue)
	{
	}
public:
	inline void init(value_type initValue)
	{
		min = initValue;
		max = initValue;
	}
	inline void update(value_type value)
	{
		if (value < min) {
			min = value;
		}
		if (value > max) {
			max = value;
		}
	}
	inline void setValue(value_type value, bool isInit)
	{
		if (isInit) {
			init(value);
		}
		else {
			update(value);
		}
	}
};

static size_t calcTKS_withSet(const std:: vector<ccfx_token_t> &seq, size_t begin, size_t end, const std::vector<ccfx_token_t> &baseTokenSet)
{
#if defined USE_BOOST_POOL
	std::set<ccfx_token_t, std::less<ccfx_token_t>, boost::fast_pool_allocator<ccfx_token_t> > tokenSet;
#else
	std::set<ccfx_token_t> tokenSet;
#endif
	tokenSet.insert(baseTokenSet.begin(), baseTokenSet.end());

	for (size_t i = begin; i < end; ++i) {
		ccfx_token_t t = seq[i];
		if (t < 0) {
			t = -1; // opened parameter token
		}
		tokenSet.insert(t);
	}

	return tokenSet.size();
}

static size_t calcTKS(const std:: vector<ccfx_token_t> &seq, size_t begin, size_t end)
{
	std::vector<ccfx_token_t> tokenSet;

	for (size_t i = begin; i < end; ++i) {
		ccfx_token_t t = seq[i];
		if (t < 0) {
			t = -1; // opened parameter token
		}
		std::vector<ccfx_token_t>::iterator it = std::lower_bound(tokenSet.begin(), tokenSet.end(), t);
		if (it == tokenSet.end() || *it != t) {
			tokenSet.insert(it, t);
			if (tokenSet.size() >= 100) {
				return calcTKS_withSet(seq, i, end, tokenSet);
			}
		}
	}

	return tokenSet.size();
}

class MetricsCalculatorError : public std::runtime_error {
public:
	MetricsCalculatorError(const std::string &message)
		: std::runtime_error(message)
	{
	}
};

class MetricsCalculator {
private:
	bool isOpen_;
	PreprocessedFileReader *pScannotner_;
	rawclonepair::RawClonePairFileAccessor *pAccessor_;
	std::string postfix_;

public:
	MetricsCalculator(PreprocessedFileReader *pScannotner, rawclonepair::RawClonePairFileAccessor *pAccessor, const std::string &postfix)
		: isOpen_(false), pScannotner_(pScannotner), pAccessor_(pAccessor), postfix_(postfix)
	{
		assert(pAccessor_ != NULL);
	}
	virtual ~MetricsCalculator()
	{
		if (isOpen_) {
			close();
		}
	}
public:
	bool isOpen() const
	{
		return isOpen_;
	}
	PreprocessedFileReader &refScannotner() const
	{
		return *pScannotner_;
	}
	rawclonepair::RawClonePairFileAccessor &refFileAccessor() const
	{
		return *pAccessor_;
	}
	std::string getPostfix() const
	{
		return postfix_;
	}
public:
	virtual void close() // called after scannotning
	{
		if (isOpen_) {
			isOpen_ = false;
		}
	}
	virtual void open(const std::string &/* fileName */) // called before file scannotning
	{
		isOpen_ = true;
	}
	virtual void scannotFile(int index) // called for each file
	{
	}
};

class FileMetricsCalculator : public MetricsCalculator {
private:
	std::string fileName;
	FileStructWrapper output;
	FILE *pOutput;
	std::vector<int> fileIDs;

	bool optionEachItem;
	bool optionSummary;

	boost::array<long, 7> fileMetricTotals; // length, #clones, NBR, RSA*LEN, RSI*LEN, CVR*LEN, (#if defined REQUIRE_RNR, then, RNR*LEN)
	MinMax<int> lengthMinMax;
	MinMax<int> countClonesMinMax;
	MinMax<int> nbrMinMax;
	MinMax<double> rsaMinMax;
	MinMax<double> rsiMinMax;
	MinMax<double> cvrMinMax;
	MinMax<double> rnrMinMax;
public:
	FileMetricsCalculator(PreprocessedFileReader *pScannotner, rawclonepair::RawClonePairFileAccessor *pAccessor, const std::string &postfix)
		: output(), pOutput(NULL), optionEachItem(false), optionSummary(false), MetricsCalculator(pScannotner, pAccessor, postfix)
	{
	}
public:
	void setOptionEachItem(bool value)
	{
		optionEachItem = value;
	}
	void setOptionSummary(bool value)
	{
		optionSummary = value;
	}
public:
	virtual void open(const std::string &fileName_) // called before file scannotning
	{
		fileName = fileName_;
		refFileAccessor().getFiles(&fileIDs);

		if (! fileName.empty()) {
			output.open(fileName.c_str(), "w");
#if defined _MSC_VER && _MSC_VER <= 1310
			if (! (bool)output) {
#else
			if (! output) {
#endif
				throw MetricsCalculatorError(std::string("can't create a file '") + fileName_ + "'");
			}
			pOutput = output.getFileStruct();
		}
		else {
			pOutput = stdout;
		}

		MetricsCalculator::open(fileName_);

		std:: fill(fileMetricTotals.begin(), fileMetricTotals.end(), 0);

#if defined REQUIRE_RNR
		fputs("FID" "\t" "LEN" "\t" "CLN" "\t" "NBR" "\t" "RSA" "\t" "RSI" "\t" "CVR" "\t" "RNR" "\n", pOutput);
#else
		fputs("FID" "\t" "LEN" "\t" "CLN" "\t" "NBR" "\t" "RSA" "\t" "RSI" "\t" "CVR" "\n", pOutput);
#endif
	}
	virtual void scannotFile(int index) // called for each file
	{
		int fileID = fileIDs[index];

		std::string errorMessage;

		boost::array<long, 7> values; // length, #clones, NBR, RSA*LEN, RSI*LEN, CVR*LEN, (#if defined REQUIRE_RNR, then, RNR*LEN)
		if (! calc_file_metrics(&values, fileID, &errorMessage)) {
			throw MetricsCalculatorError(errorMessage);
		}
		for (size_t fi = 0; fi < 7; ++fi) {
			fileMetricTotals[fi] += values[fi];
		}
		if (optionEachItem) {
			if (values[0] != 0) {
				std::string s = (
#if defined REQUIRE_RNR
						boost::format("%d\t%d\t%d\t%d\t%g\t%g\t%g\t%g" "\n") 
#else
						boost::format("%d\t%d\t%d\t%d\t%g\t%g\t%g" "\n") 
#endif
						% fileID 
						% (int)values[0] % (int)values[1] % (int)values[2]
						% (values[3] / (double)values[0]) % (values[4] / (double)values[0]) % (values[5] / (double)values[0])
#if defined REQUIRE_RNR
						% (values[6] / (double)values[0])
#endif
						).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
			}
			else {
				std::string s = (
#if defined REQUIRE_RNR
						boost::format("%d\t0\t0\t0\t0\t0\t0\t1.0" "\n")
#else
						boost::format("%d\t0\t0\t0\t0\t0\t0" "\n")
#endif
						% fileID).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
			}
		}
		const int i = index;
		if (values[0] != 0) {
			lengthMinMax.setValue(values[0], i == 0);
			countClonesMinMax.setValue(values[1], i == 0);
			nbrMinMax.setValue(values[2], i == 0);
			rsaMinMax.setValue(values[3] / (double)values[0], i == 0);
			rsiMinMax.setValue(values[4] / (double)values[0], i == 0);
			cvrMinMax.setValue(values[5] / (double)values[0], i == 0);
#if defined REQUIRE_RNR
			rnrMinMax.setValue(values[6] / (double)values[0], i == 0);
#endif
		}
		else {
			lengthMinMax.setValue(0, i == 0);
			countClonesMinMax.setValue(0, i == 0);
			nbrMinMax.setValue(0, i == 0);
			rsaMinMax.setValue(0.0, i == 0);
			rsiMinMax.setValue(0.0, i == 0);
			cvrMinMax.setValue(0.0, i == 0);
#if defined REQUIRE_RNR
			rnrMinMax.setValue(1.0, i == 0);
#endif
		}
	}
	virtual void close() // called after scannotning
	{
		if (optionSummary) {
			std::string s = (
#if defined REQUIRE_RNR
					boost::format("ave.\t%g\t%g\t%g\t%g\t%g\t%g\t%g" "\n") 
#else
					boost::format("ave.\t%g\t%g\t%g\t%g\t%g\t%g" "\n") 
#endif
					% (fileIDs.size() != 0 ? fileMetricTotals[0] / (double)fileIDs.size() : 0.0)
					% (fileIDs.size() != 0 ? fileMetricTotals[1] / (double)fileIDs.size() : 0.0)
					% (fileIDs.size() != 0 ? fileMetricTotals[2] / (double)fileIDs.size() : 0.0)
					% (fileMetricTotals[0] != 0 ? fileMetricTotals[3] / (double)fileMetricTotals[0] : 0.0)
					% (fileMetricTotals[0] != 0 ? fileMetricTotals[4] / (double)fileMetricTotals[0] : 0.0)
					% (fileMetricTotals[0] != 0 ? fileMetricTotals[5] / (double)fileMetricTotals[0] : 0.0)
#if defined REQUIRE_RNR
					% (fileMetricTotals[0] != 0 ? fileMetricTotals[6] / (double)fileMetricTotals[0] : 0.0)
#endif
					).str();
			FWRITEBYTES(s.data(), s.length(), pOutput);
			s = (
#if defined REQUIRE_RNR
					boost::format("min.\t%d\t%d\t%d\t%g\t%g\t%g\t%g" "\n") 
#else
					boost::format("min.\t%d\t%d\t%d\t%g\t%g\t%g" "\n") 
#endif
					% lengthMinMax.min % countClonesMinMax.min % nbrMinMax.min
					% rsaMinMax.min % rsiMinMax.min % cvrMinMax.min 
#if defined REQUIRE_RNR
					% rnrMinMax.min
#endif
					).str();
			FWRITEBYTES(s.data(), s.length(), pOutput);
			s = (
#if defined REQUIRE_RNR
					boost::format("max.\t%d\t%d\t%d\t%g\t%g\t%g\t%g" "\n") 
#else
					boost::format("max.\t%d\t%d\t%d\t%g\t%g\t%g" "\n") 
#endif
					% lengthMinMax.max % countClonesMinMax.max % nbrMinMax.max
					% rsaMinMax.max % rsiMinMax.max % cvrMinMax.max 
#if defined REQUIRE_RNR
					% rnrMinMax.max
#endif
					).str();

			FWRITEBYTES(s.data(), s.length(), pOutput);
		}

		if (! fileName.empty()) {
			output.close();
		}

		fileName.clear();
		pOutput = NULL;
	}
private:
	bool calc_file_metrics(
		boost::array<long, 7> *pValues, // length, #clones, NBR, RSA*LEN, RSI*LEN, CVR*LEN (#f defined REQUIRE_RNR, then, RNR*LEN)
		int fileID,
		std::string *pErrorMessage
	)
	{
		rawclonepair::RawClonePairFileAccessor &acc = refFileAccessor();

		boost::array<long, 7> &values = *pValues; // length, #clones, NBR, RSA*LEN, RSI*LEN, CVR*LEN, (#if defined REQUIRE_RNR, then, RNR*LEN)
		
		// LEN
		size_t len;
		std:: string fileName;
		acc.getFileDescription(fileID, &fileName, &len);
		fileName = INNER2SYS(fileName);

		// #CLONE, RSA_LEN, RSI_LEN, #NEIGHBOR
		if (len > 0) {
			boost::dynamic_bitset<> tokensCoveredByOthers;
			tokensCoveredByOthers.resize(len);
			boost::dynamic_bitset<> tokensCoveredBySelf;
			tokensCoveredBySelf.resize(len);
			std::vector<boost::uint64_t> cloneClassIDs;
			std::vector<int> filesHavingCloneWithIt;
			std:: vector<rawclonepair::RawClonePair> clonePairs;
			acc.getRawClonePairsOfFile(fileID, &clonePairs);
			for (size_t i = 0; i < clonePairs.size(); ++i) {
				const rawclonepair::RawClonePair &pair = clonePairs[i];
				assert(pair.left.file == fileID);
				std::vector<boost::uint64_t>::iterator it = std::lower_bound(cloneClassIDs.begin(), cloneClassIDs.end(), pair.reference);
				if (it == cloneClassIDs.end() || *it != pair.reference) {
					cloneClassIDs.insert(it, pair.reference);
				}
				if (pair.right.file != pair.left.file) {
					std::vector<int>::iterator it = std::lower_bound(filesHavingCloneWithIt.begin(), filesHavingCloneWithIt.end(), pair.right.file);
					if (it == filesHavingCloneWithIt.end() || *it != pair.right.file) {
						filesHavingCloneWithIt.insert(it, pair.right.file);
					}
				}
				if (pair.right.file != fileID) {
					assert(0 <= pair.left.begin && pair.left.begin <= pair.left.end && pair.left.end <= tokensCoveredByOthers.size());
					for (size_t i = pair.left.begin; i < pair.left.end; ++i) {
						tokensCoveredByOthers.set(i, true);
					}
				}
				else {
					assert(0 <= pair.left.begin && pair.left.begin <= pair.left.end && pair.left.end <= tokensCoveredBySelf.size());
					for (size_t i = pair.left.begin; i < pair.left.end; ++i) {
						tokensCoveredBySelf.set(i, true);
					}
				}
			}
			
			boost::dynamic_bitset<> tokensCoveredByAny = tokensCoveredByOthers | tokensCoveredBySelf;
			values[0] = len;
			values[1] = cloneClassIDs.size();
			values[2] = filesHavingCloneWithIt.size();
			values[3] = tokensCoveredByOthers.count();
			values[4] = tokensCoveredBySelf.count();
			values[5] = tokensCoveredByAny.count();
		} 
		else {
			std:: fill(values.begin(), values.begin() + 6, 0);
		}

#if defined REQUIRE_RNR
		if (len == 0) {
			values[6] = 0;
		}
		else if (len == 1) {
			values[6] = 1;
		}
		else {
			// RNR
			std:: vector<ccfx_token_t> seq;
			std:: string errorMessage;
			if (! getPreprocessedSequenceOfFile(&seq, fileName, getPostfix(), &refScannotner(), &errorMessage)) {
				*pErrorMessage = errorMessage;
				return false;
			}
			std::vector<repdet::Repetition> reps;
			repdet::RepetitionDetector<ccfx_token_t>().findRepetitions(&reps, seq, 0);
			boost::dynamic_bitset<> tokensRepeated;
			const int shift_by_first_zero = 1; // this shift caused by PreprocessedFileReader::readFileget, which is callded via PreprocessedSequenceOfFile
			assert(seq.size() == len + shift_by_first_zero);
			tokensRepeated.resize(seq.size());
			for (size_t i = 0; i < reps.size(); ++i) {
				const repdet::Repetition &rep = reps[i];
				assert(0 <= rep.beginEnd.first + rep.unit && rep.beginEnd.first + rep.unit <= rep.beginEnd.second && rep.beginEnd.second <= tokensRepeated.size());
				for (size_t pos = rep.beginEnd.first + rep.unit; pos < rep.beginEnd.second; ++pos) {
					tokensRepeated[pos] = true;
				}
			}
			size_t countOfTokensRepeated = tokensRepeated.count();
			assert(countOfTokensRepeated + shift_by_first_zero <= seq.size());
			values[6] = (seq.size() - shift_by_first_zero) - countOfTokensRepeated;
		}
#endif
		return true;
	}
};

class CloneMetricsCalculator : public MetricsCalculator {
private:
	struct RNRTKS {
		bool available;
		size_t rnr;
		size_t tks;
		size_t loop;
		size_t cond;
	public:
		RNRTKS()
			: available(false)
		{
		}
	};

private:
	std::string fileName;
	FileStructWrapper output;
	FILE *pOutput;
	std::vector<int> fileIDs;

	bool optionEachItem;
	bool optionSummary;

	FILE *pTemp;
	std::string tempFileName;
public:
	CloneMetricsCalculator(PreprocessedFileReader *pScannotner, rawclonepair::RawClonePairFileAccessor *pAccessor, const std::string &postfix)
		: pOutput(NULL), optionEachItem(false), optionSummary(false), pTemp(NULL), MetricsCalculator(pScannotner, pAccessor, postfix)
	{
	}
public:
	void setOptionEachItem(bool value)
	{
		optionEachItem = value;
	}
	void setOptionSummary(bool value)
	{
		optionSummary = value;
	}
public:
	virtual void open(const std::string &fileName_) // called before file scannotning
	{
		fileName = fileName_;
		refFileAccessor().getFiles(&fileIDs);

		if (! fileName.empty()) {
			output.open(fileName.c_str(), "w");
#if defined _MSC_VER && _MSC_VER <= 1310
			if (! (bool)output) {
#else
			if (! output) {
#endif
				throw MetricsCalculatorError(std::string("can't create a file '") + fileName_ + "'");
			}
			pOutput = output.getFileStruct();
		}
		else {
			pOutput = stdout;
		}

		MetricsCalculator::open(fileName_);

		pTemp = NULL;
		if (! prepare_RNRTKS_tempdata(&pTemp, &tempFileName)) {
			throw MetricsCalculatorError("can't create a temporary file (3)");
		}
	}
	virtual void scannotFile(int index) // called for each file
	{
		rawclonepair::RawClonePairFileAccessor &acc = refFileAccessor();

		int fileID = fileIDs[index];

		std:: string fileName;
		size_t length;
		acc.getFileDescription(fileID, &fileName, &length);
		fileName = INNER2SYS(fileName);

		std:: vector<rawclonepair::RawClonePair> clonePairs;
		acc.getRawClonePairsOfFile(fileID, &clonePairs);
		
		std:: vector<ccfx_token_t> seq;
		boost::dynamic_bitset<> tokensRepeated;
		for (size_t j = 0; j < clonePairs.size(); ++j) {
			assert(clonePairs[j].left.file == fileID);

			boost::uint64_t cid = clonePairs[j].reference;
			RNRTKS rnr;
			FSEEK64(pTemp, cid * sizeof(RNRTKS), SEEK_SET);
			FREAD(&rnr, sizeof(RNRTKS), 1, pTemp);
			if (! rnr.available) {
				if (seq.size() == 0) {
					std:: string errorMessage;
					if (! getPreprocessedSequenceOfFile(&seq, fileName, getPostfix(), &refScannotner(), &errorMessage)) {
						throw MetricsCalculatorError(errorMessage);
					}
					remove_displacement<std:: vector<ccfx_token_t>::iterator>(seq.begin(), seq.end());
				}

				const int shift_by_first_zero = 1; // this shift caused by PreprocessedFileReader::readFileget, which is callded via PreprocessedSequenceOfFile

				const rawclonepair::RawFileBeginEnd &rfbe = clonePairs[j].left;
				
				// calc rnr
				{
					size_t seqSize = seq.size();
					if (! (rfbe.end + shift_by_first_zero <= seqSize)) {
						throw MetricsCalculatorError("the clone data file may be obsolete.");
					}
					size_t overlappedSize = 0;
					const rawclonepair::RawFileBeginEnd &r = clonePairs[j].right;
					bool tailOverlap = false;
					if (r.file == rfbe.file) {
						if (r.begin <= rfbe.begin && rfbe.begin <= r.end) {
							overlappedSize = r.end - rfbe.begin;
							tailOverlap = false;
						}
						else if (rfbe.begin <= r.begin && r.begin < rfbe.end) {
							overlappedSize = rfbe.end - r.begin;
							tailOverlap = true;
						}
						assert(overlappedSize < rfbe.end - rfbe.begin);
						if (overlappedSize < (rfbe.end - rfbe.begin) / 2) {
							overlappedSize = 0;
						}
					}
					std:: vector<repdet::Repetition> reps;
					tokensRepeated.clear();
					tokensRepeated.resize(seq.size(), false);
					if (tailOverlap) {
						repdet::RepetitionDetector<ccfx_token_t>().findRepetitions(&reps, seq, 
							rfbe.begin + shift_by_first_zero, rfbe.end - overlappedSize + shift_by_first_zero, 0);
					}
					else {
						repdet::RepetitionDetector<ccfx_token_t>().findRepetitions(&reps, seq, 
							rfbe.begin + shift_by_first_zero + overlappedSize, rfbe.end + shift_by_first_zero, 0);
					}
					for (std:: vector<repdet::Repetition>::const_iterator i = reps.begin(); i != reps.end(); ++i) {
						repdet::Repetition rep = *i;
						rep.beginEnd.first -= shift_by_first_zero;
						rep.beginEnd.second -= shift_by_first_zero;
						assert(rfbe.begin <= rep.beginEnd.first + rep.unit && rep.beginEnd.first + rep.unit <= rep.beginEnd.second && rep.beginEnd.second <= rfbe.end);
						for (size_t pos = rep.beginEnd.first + rep.unit; pos < rep.beginEnd.second; ++pos) {
							tokensRepeated.set(pos, true);
						}
					}
					size_t count = tokensRepeated.count() + overlappedSize;
					rnr.rnr= (rfbe.end - rfbe.begin) - count;
				}

				// calc tks
				rnr.tks = calcTKS(seq, rfbe.begin + shift_by_first_zero, rfbe.end + shift_by_first_zero);

				// calc loop, cond
				ccfx_token_t code_c_loop = refScannotner().getCode("c_loop");
				rnr.loop = std:: count(seq.begin() + rfbe.begin + shift_by_first_zero, seq.begin() + rfbe.end + shift_by_first_zero, code_c_loop);
				ccfx_token_t code_c_cond = refScannotner().getCode("c_cond");
				rnr.cond = std:: count(seq.begin() + rfbe.begin + shift_by_first_zero, seq.begin() + rfbe.end + shift_by_first_zero, code_c_cond);

				rnr.available = true;

				FSEEK64(pTemp, cid * sizeof(RNRTKS), SEEK_SET);
				FWRITE(&rnr, sizeof(RNRTKS), 1, pTemp);
			}
		}
	}
	virtual void close() // called after scannotning
	{
		rawclonepair::RawClonePairFileAccessor &acc = refFileAccessor();

		boost::array<long, 9> cloneMetricTotals; // len, pop, nif, rad, rnr, tks, loop, cond, cyclomatic
		MinMax<int> lenMinMax;
		MinMax<int> popMinMax;
		MinMax<int> nifMinMax;
		MinMax<int> radMinMax;
		MinMax<double> rnrMinMax;
		MinMax<int> tksMinMax;
		MinMax<int> loopMinMax;
		MinMax<int> condMinMax;
		MinMax<int> cyclomaticMinMax;

		std:: fill(cloneMetricTotals.begin(), cloneMetricTotals.end(), 0);

		fputs("CID" "\t" "LEN" "\t" "POP" "\t" "NIF" "\t" "RAD" "\t" "RNR" "\t" "TKS" "\t" "LOOP" "\t" "COND" "\t" "McCabe" "\n", pOutput);
		
		boost::uint64_t cidCount = 0;
		boost::uint64_t cid;
		if (acc.getFirstCloneSetID(&cid)) {
			bool firstOne = true;
			while (true) {
				++cidCount;

				std:: vector<rawclonepair::RawFileBeginEnd> codeFragments;
				acc.getCodeFragmentsOfCloneSet(cid, &codeFragments);
				if (! codeFragments.empty()) {
					const rawclonepair::RawFileBeginEnd &f0 = codeFragments[0];
					int cloneMetricValues[9]; // len, pop, nif, rad, rnr, tks, loop, cond, cyclomatic
					int &len = cloneMetricValues[0];
					int &pop = cloneMetricValues[1];
					int &nif = cloneMetricValues[2];
					int &rad = cloneMetricValues[3];
					int &rnr = cloneMetricValues[4];
					int &tks = cloneMetricValues[5];
					int &loop = cloneMetricValues[6];
					int &cond = cloneMetricValues[7];
					int &cyclomatic = cloneMetricValues[8];

					len = f0.end - f0.begin;

					pop = codeFragments.size();
					
					nif = 0;
					rad = 0;
					{
						std:: vector<int> fileIDs;
						for (size_t i = 0; i < codeFragments.size(); ++i) {
							fileIDs.push_back(codeFragments[i].file);
						}
						std:: sort(fileIDs.begin(), fileIDs.end());
						std:: vector<int>::iterator it = std::unique(fileIDs.begin(), fileIDs.end());
						fileIDs.resize(it - fileIDs.begin());
						nif = fileIDs.size();
						if (fileIDs.size() == 1) {
							rad = 0;
						}
						else {
							std:: vector<int>::const_iterator j = fileIDs.begin();
							if (j != fileIDs.end()) {
								int fileID = *j;
								std:: string fileName;
								acc.getFileDescription(fileID, &fileName, NULL);
								std:: string commonPath = fileName;
								for (++j; j != fileIDs.end(); ++j) {
									int fileID = *j;
									acc.getFileDescription(fileID, &fileName, NULL);
									commonPath = common_prefix(commonPath, fileName);
								}
								int remainingLengthMax = 0;
								int pathSepCountMax = 0;
								for (j = fileIDs.begin(); j != fileIDs.end(); ++j) {
									int fileID = *j;
									acc.getFileDescription(fileID, &fileName, NULL);
									std:: string r = fileName.substr(commonPath.length());
									if (r.length() > remainingLengthMax) {
										remainingLengthMax = r.length();
									}
									int pathSepCount = count_char(r, '\\');
									int pathSepCount2 = count_char(r, '/');
									if (pathSepCount2 > pathSepCount) {
										pathSepCount = pathSepCount2;
									}
									if (pathSepCount > pathSepCountMax) {
										pathSepCountMax = pathSepCount;
									}
								}
								if (remainingLengthMax == 0) {
									rad = 1;
								}
								else {
									rad = pathSepCountMax + 1;
								}
							}
						}
					}
					
					FSEEK64(pTemp, cid * sizeof(RNRTKS), SEEK_SET);
					RNRTKS rnrtks;
					FREAD(&rnrtks, sizeof(RNRTKS), 1, pTemp);
					if (rnrtks.available) {
						rnr = rnrtks.rnr;
						tks = rnrtks.tks;
						loop = rnrtks.loop;
						cond = rnrtks.cond;
						cyclomatic = loop + cond;
					}
					else {
						rnr = 0;
						tks = 0;
						loop = 0;
						cond = 0;
						cyclomatic = 0;
					}
					if (optionEachItem) {
						std::string s = (boost::format("%ld\t%d\t%d\t%d\t%d\t%g\t%d\t%d\t%d\t%d" "\n") 
								% (boost::uint64_t)cid 
								% (int)len 
								% (int)pop 
								% (int)nif
								% (int)rad
								% (len != 0 ? (rnr / (double)len) : 0.0)
								% tks
								% loop
								% cond
								% cyclomatic
								).str();
						FWRITEBYTES(s.data(), s.length(), pOutput);
					}
					for (size_t i = 0; i < 9; ++i) {
                        cloneMetricTotals[i] += cloneMetricValues[i];
					}
					lenMinMax.setValue(len, firstOne);
					popMinMax.setValue(pop, firstOne);
					nifMinMax.setValue(nif, firstOne);
					radMinMax.setValue(rad, firstOne);
					rnrMinMax.setValue(len != 0 ? (rnr / (double)len) : 0.0, firstOne);
					tksMinMax.setValue(tks, firstOne);
					loopMinMax.setValue(loop, firstOne);
					condMinMax.setValue(cond, firstOne);
					cyclomaticMinMax.setValue(cyclomatic, firstOne);
				}
				else {
					assert(false);
				}
				if (! acc.getNextCloneSetID(&cid)) {
					break;
				}
				firstOne = false;
			}
		}
		if (optionSummary) {
			if (cidCount > 0) {
				std::string s = (boost::format("ave.\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g" "\n") 
						% (cloneMetricTotals[0] / (double)cidCount)
						% (cloneMetricTotals[1] / (double)cidCount)
						% (cloneMetricTotals[2] / (double)cidCount)
						% (cloneMetricTotals[3] / (double)cidCount)
						% (cloneMetricTotals[0] != 0 ? cloneMetricTotals[4] / (double)cloneMetricTotals[0] : 0.0)
						% (cloneMetricTotals[5] / (double)cidCount)
						% (cloneMetricTotals[6] / (double)cidCount)
						% (cloneMetricTotals[7] / (double)cidCount)
						% (cloneMetricTotals[8] / (double)cidCount)
						).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
				s = (boost::format("min.\t%d\t%d\t%d\t%d\t%g\t%d\t%d\t%d\t%d" "\n") 
						% lenMinMax.min % popMinMax.min % nifMinMax.min % radMinMax.min % rnrMinMax.min
						% tksMinMax.min % loopMinMax.min % condMinMax.min % cyclomaticMinMax.min
						).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
				s = (boost::format("max.\t%d\t%d\t%d\t%d\t%g\t%d\t%d\t%d\t%d" "\n") 
						% lenMinMax.max % popMinMax.max % nifMinMax.max % radMinMax.max % rnrMinMax.max
						% tksMinMax.max % loopMinMax.max % condMinMax.max % cyclomaticMinMax.max
						).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
			}
			else {
				fputs("ave.\t0\t0\t0\t0\t0\t0\t0\t0\t0" "\n", pOutput);
				fputs("min.\t0\t0\t0\t0\t0\t0\t0\t0\t0" "\n", pOutput);
				fputs("max.\t0\t0\t0\t0\t0\t0\t0\t0\t0" "\n", pOutput);
			}
		}

		if (pTemp != NULL) {
			fclose(pTemp);
			remove(tempFileName.c_str());
		}
	}
private:
	bool prepare_RNRTKS_tempdata(FILE **ppTemp, std:: string *pTempFileName)
	{
		rawclonepair::RawClonePairFileAccessor &acc = refFileAccessor();

		boost::uint64_t maxCid = acc.getMaxCloneSetID();
		std:: string tempFileRaw = ::make_temp_file_on_the_same_directory(fileName, "ccfxclonemetric", ".tmp");
		FILE *pTemp = fopen(tempFileRaw.c_str(), "w+b" F_TEMPORARY_FILE_OPTIMIZATION);
		if (pTemp == NULL) {
			std:: cerr << "error: can't create a temporary file (4)" << std:: endl;
			return false;
		}
		RNRTKS rnr;
		for (boost::uint64_t cid = 0; cid <= maxCid; ++cid) {
			FWRITE(&rnr, sizeof(RNRTKS), 1, pTemp);
		}

		*ppTemp = pTemp;
		*pTempFileName = tempFileRaw;

		return true;
	}

	static std:: string common_prefix(const std:: string &s, const std:: string &t)
	{
		size_t i = 0;
		while (i < s.length() && i < t.length()) {
			if (s[i] != t[i]) {
				break;
			}
			++i;
		}
		return s.substr(0, i);
	}

	static size_t count_char(const std:: string &str, char ch)
	{
		size_t count = 0;
		for (size_t i = 0; i < str.length(); ++i) {
			if (str[i] == ch) {
				++count;
			}
		}
		return count;
	}

};

class LinebasedMetricsCalculator : public MetricsCalculator {
private:
	std::string fileName;
	FileStructWrapper output;
	FILE *pOutput;
	std::vector<int> fileIDs;

	bool optionEachItem;
	bool optionSummary;

	boost::array<long, 3> lineMetricTotals;
	metrics::MinMax<long> locMinMax;
	metrics::MinMax<long> slocMinMax;
	metrics::MinMax<long> clocMinMax;
	metrics::MinMax<double> cvrlMinMax;
public:
	LinebasedMetricsCalculator(PreprocessedFileReader *pScannotner, rawclonepair::RawClonePairFileAccessor *pAccessor, const std::string &postfix)
		: pOutput(NULL), optionEachItem(false), optionSummary(false), MetricsCalculator(pScannotner, pAccessor, postfix)
	{
	}
public:
	void setOptionEachItem(bool value)
	{
		optionEachItem = value;
	}
	void setOptionSummary(bool value)
	{
		optionSummary = value;
	}
public:
	virtual void open(const std::string &fileName_) // called before file scannotning
	{
		fileName = fileName_;
		refFileAccessor().getFiles(&fileIDs);

		if (! fileName.empty()) {
			output.open(fileName.c_str(), "w");
			if (! output) {
				throw MetricsCalculatorError(std::string("can't create a file '") + fileName_ + "'");
			}
			pOutput = output.getFileStruct();
		}
		else {
			pOutput = stdout;
		}

		MetricsCalculator::open(fileName_);

		std::fill(lineMetricTotals.begin(), lineMetricTotals.end(), 0);
		
		fputs("FID" "\t" "LOC" "\t" "SLOC" "\t" "CLOC" "\t" "CVRL" "\n", pOutput);
	}
	virtual void scannotFile(int index)
	{
		int fileID = fileIDs[index];

		boost::array<long, 3> lineMetrics;
		calc_wordcount(fileID, &lineMetrics);
		double cvrl = lineMetrics[1] != 0 ? lineMetrics[2] / (double)lineMetrics[1] : 0.0;
		if (optionEachItem) {
			std::string s = (boost::format("%d\t%d\t%d\t%d\t%g" "\n") 
					% fileID 
					% (int)lineMetrics[0]
					% (int)lineMetrics[1]
					% (int)lineMetrics[2]
					% cvrl
					).str();
			FWRITEBYTES(s.data(), s.length(), pOutput);
		}
		int i = index;
		if (i == 0) {
			locMinMax.init(lineMetrics[0]);
			slocMinMax.init(lineMetrics[1]);
			clocMinMax.init(lineMetrics[2]);
			cvrlMinMax.init(cvrl);
		}
		else {
			locMinMax.update(lineMetrics[0]);
			slocMinMax.update(lineMetrics[1]);
			clocMinMax.update(lineMetrics[2]);
			cvrlMinMax.update(cvrl);
		}
		lineMetricTotals[0] += lineMetrics[0];
		lineMetricTotals[1] += lineMetrics[1];
		lineMetricTotals[2] += lineMetrics[2];
	}
	virtual void close() // called after scannotning
	{
		if (optionSummary) {
			if (fileIDs.size() != 0) {
				double count = fileIDs.size();
				double cvrl = lineMetricTotals[1] != 0 ? lineMetricTotals[2] / (double)lineMetricTotals[1] : 0.0;
				std::string s = (boost::format("total\t%d\t%d\t%d\t-" "\n") % lineMetricTotals[0] % lineMetricTotals[1] % lineMetricTotals[2]).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
				s = (boost::format("ave.\t%g\t%g\t%g\t%g" "\n") % (lineMetricTotals[0] / count) % (lineMetricTotals[1] / count) % (lineMetricTotals[2] / count) % cvrl).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
				s = (boost::format("min.\t%d\t%d\t%d\t%g" "\n") % locMinMax.min % slocMinMax.min % clocMinMax.min % cvrlMinMax.min).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
				s = (boost::format("max.\t%d\t%d\t%d\t%g" "\n") % locMinMax.max % slocMinMax.max % clocMinMax.max % cvrlMinMax.max).str();
				FWRITEBYTES(s.data(), s.length(), pOutput);
			}
			else {
				fputs("total\t0\t0\t0\t0" "\n", pOutput);
				fputs("ave.\t0\t0\t0\t0" "\n", pOutput);
				fputs("min.\t0\t0\t0\t0" "\n", pOutput);
				fputs("max.\t0\t0\t0\t0" "\n", pOutput);
			}
		}
	}
private:
	void calc_wordcount(int fileID, boost::array<long, 3> *pLineMetrics)
	{
		rawclonepair::RawClonePairFileAccessor &acc = refFileAccessor();
		std::string postfix = getPostfix();

		size_t len;
		std:: string fileName;
		acc.getFileDescription(fileID, &fileName, &len);
		fileName = INNER2SYS(fileName);

		boost::dynamic_bitset<> tokensCoveredByClones;
		{
			tokensCoveredByClones.resize(len, false);
			std:: vector<rawclonepair::RawClonePair> clonePairs;
			acc.getRawClonePairsOfFile(fileID, &clonePairs);
			for (size_t i = 0; i < clonePairs.size(); ++i) {
				const rawclonepair::RawClonePair &pair = clonePairs[i];
				assert(pair.left.file == fileID);
				assert(0 <= pair.left.begin && pair.left.begin <= pair.left.end && pair.left.end <= tokensCoveredByClones.size());
				for (size_t i = pair.left.begin; i < pair.left.end; ++i) {
					tokensCoveredByClones.set(i, true);
				}
			}
		}

		size_t loc;
		size_t sloc;
		size_t coveredLoc;
		if (! refScannotner().countLinesOfFile(fileName, postfix,
				&loc, &sloc, &coveredLoc, &tokensCoveredByClones)) {
			throw MetricsCalculatorError(std:: string("can't open a preprocessed file of '") + fileName + "' (#6)");
		}
		(*pLineMetrics)[0] = loc;
		(*pLineMetrics)[1] = sloc;
		(*pLineMetrics)[2] = coveredLoc;
	}
};

}; // namespace metrics

class MetricMain {
private:
	std:: string inputFile;
	std:: string cloneMetricOutputFile;
	std:: string fileMetricOutputFile;
	std:: string wordcountOutputFile;
	bool cloneMetricRequired;
	bool fileMetricRequired;
	bool wordcountRequired;
	bool optionSummary;
	bool optionEachItem;

	PreprocessedFileReader scannotner;
	rawclonepair::RawClonePairFileAccessor acc;
	std::string postfix;

	Decoder defaultDecoder;

	ThreadFunction threadFunction;

private:
	std:: string SYS2INNER(const std::string &systemString)
	{
		return toUTF8String(defaultDecoder.decode(systemString));
	}
	std:: string INNER2SYS(const std::string &innerString)
	{
		return defaultDecoder.encode(toWStringV(innerString));
	}

public:
	int main(const std::vector<std::string> &argv) 
	{
		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std:: cout << 
				"Usage: ccfx M in.ccfxd [-c [-o out]] [-f [-o out]] [-w [-o out]] [-p FIELDS]" "\n"
				"  Calculates clone metrics and/or file metrics from a clone data file." "\n"
				"Options" "\n"
				"  -c: calculates clone metrics LEN, POP, NIF, RAD, RNR, TKS, COND, LOOP, and McCabe." "\n"
				"  -f: calculates file metrics LEN, CLN, NBR, RSA, RSI, CVR, and RNR." "\n"
				//"  -n dir: specify directory where preprocessed files are created." "\n"
				"  -o filename: output file name." "\n"
				"  -p s: prints out average, min. and max. of each metric." "\n"
				"  -p i-: don't print metric values for each file / clone set." "\n"
				"  -w: calculates line-based metrics LOC, SLOC, CLOC, CVRL." "\n"
				"  --threads=number: max working threads (0)." "\n"
				;
			return 0;
		}

		fileMetricRequired = false;
		cloneMetricRequired = false;
		wordcountRequired = false;
		optionSummary = false;
		optionEachItem = true;

		enum { NONE_MODE, FILE_MODE, CLONE_MODE, WORDCOUNT_MODE } mode = NONE_MODE;

		for (size_t i = 2; i < argv.size(); ++i) {
			std:: string argi = argv[i];
			if (boost::starts_with(argi, "-")) {
				if (argi == "-f") {
					mode = FILE_MODE;
					fileMetricRequired = true;
				}
				else if (argi == "-c") {
					mode = CLONE_MODE;
					cloneMetricRequired = true;
				}
				else if (argi == "-w") {
					mode = WORDCOUNT_MODE;
					wordcountRequired = true;
				}
				else if (argi == "-o") {
					switch (mode) {
					case FILE_MODE:
						{
							if (! (i + 1 < argv.size())) {
								std:: cerr << "error: option -o requires an argument" << std:: endl;
								return 1;
							}
							fileMetricOutputFile = argv[i + 1];
							++i;
						}
						break;
					case CLONE_MODE:
						{
							if (! (i + 1 < argv.size())) {
								std:: cerr << "error: option -o requires an argument" << std:: endl;
								return 1;
							}
							cloneMetricOutputFile = argv[i + 1];
							++i;
						}
						break;
					case WORDCOUNT_MODE:
						{
							if (! (i + 1 < argv.size())) {
								std:: cerr << "error: option -o requires an argument" << std:: endl;
								return 1;
							}
							wordcountOutputFile = argv[i + 1];
							++i;
						}
						break;
					default:
						{
							std:: cerr << "error: mode specification -c, -f, or -w is required before option -o" << std:: endl;
							return 1;
						}
						break;
					}
				}
				else if (argi == "-v") {
					// option -v is obsolete
				}
				else if (argi == "-p") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -p requires an argument" << std:: endl;
						return 1;
					}
					std:: string arg = argv[i + 1];
					for (size_t j = 0; j < arg.length(); ++j) {
						int ch = arg[j];
						switch (ch) {
						case 's':
							if (j + 1 < arg.length() && arg[j + 1] == '-') {
								optionSummary = false;
								++j;
							}
							else {
								optionSummary = true;
							}
							break;
						case 'i':
							if (j + 1 < arg.length() && arg[j + 1] == '-') {
								optionEachItem = false;
								++j;
							}
							else {
								optionEachItem = true;
							}
							break;
						default:
							std:: cerr << "error: invalid argument for option -p" << std:: endl;
							return 1;
						}
					}
					++i;
				}
				//else if (argi == "-n") {
				//	if (! (i + 1 < argv.size())) {
				//		std:: cerr << "error: option -n requires an argument" << std:: endl;
				//		return 1;
				//	}
				//	std:: string arg = argv[i + 1];
				//	rawReader.addPreprocessFileDirectory(arg);
				//	++i;
				//}
				else {
					std::pair<int, std::string> r = threadFunction.scanOption(argi, (i + 1 < argv.size()) ? argv[i + 1] : "");
					if (r.first > 0) {
						i += r.first - 1;
					}
					else if (r.first < 0) {
						std::cerr << "error: " << r.second << std::endl;
						return 1;
					}
					else {
						std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
						return 1;
					}
				}
			}
			else {
				if (inputFile.empty()) {
					inputFile = argi;
					boost::array<std::string, 3> exts = {{ pairBinaryDiploidExtension, ".icetd", ".tmp" }};
					force_extension(&inputFile, exts.begin(), exts.end());
				}
				else {
					std:: cerr << "error: too many command-line arguments" << std:: endl;
					return 1;
				}
			}
		}

		if (inputFile.empty()) {
			inputFile = "a" + pairBinaryDiploidExtension;
		}

		if (! fileMetricRequired && ! cloneMetricRequired && ! wordcountRequired) {
			std:: cerr << "error: no metric to be calculated" << std:: endl;
			return 1;
		}
		
		threadFunction.applyToSystem();

		size_t numOfMetricsOutputForStdout = 0;
		if (fileMetricRequired && fileMetricOutputFile.empty()) {
			++numOfMetricsOutputForStdout;
		}
		if (cloneMetricRequired && cloneMetricOutputFile.empty()) {
			++numOfMetricsOutputForStdout;
		}
		if (wordcountRequired && wordcountOutputFile.empty()) {
			++numOfMetricsOutputForStdout;
		}
		if (numOfMetricsOutputForStdout >= 2) {
			std:: cerr << "error: two or more metrics is specified to print out to the standard output" << std:: endl;
			return 1;
		}

		try {
			return do_calculation(inputFile, cloneMetricOutputFile, fileMetricOutputFile, wordcountOutputFile);
		}
		catch (metrics::MetricsCalculatorError &e) {
			std::cerr << "error: " << e.what() << std::endl;
			return 1;
		}
	}
	
	int do_calculation(const std:: string &inputFile, 
			const std::string &cloneMetricOutputFile, const std::string &fileMetricOutputFile, const std::string &wordcountOutputFile)
	{
		PreprocessedFileRawReader rawReader;

		if (! acc.open(inputFile, 
				rawclonepair::RawClonePairFileAccessor::CLONEDATA 
				| rawclonepair::RawClonePairFileAccessor::FILEDATA)) {
			std:: cerr << "error: " << acc.getErrorMessage() << std:: endl;
			return 1;
		}
		acc.setCacheUsage(true);

		std::vector<std::string> p = acc.getOptionValues(PREPROCESSED_FILE_POSTFIX);
		if (! p.empty()) {
			postfix = p.back();
		}
		else {
			postfix = "." + acc.getPreprocessScript() + ".ccfxprep";
		}

		{
			std::vector<std::string> prepDirs = acc.getOptionValues("n");
			for (std::vector<std::string>::iterator pi = prepDirs.begin(); pi != prepDirs.end(); ++pi) {
				*pi = INNER2SYS(*pi);
			}
			rawReader.setPreprocessFileDirectories(prepDirs);
			scannotner.setRawReader(rawReader);
		}

		metrics::FileMetricsCalculator fmc(&scannotner, &acc, postfix);
		if (fileMetricRequired) {
			fmc.setOptionEachItem(optionEachItem);
			fmc.setOptionSummary(optionSummary);
			fmc.open(fileMetricOutputFile);
		}

		metrics::CloneMetricsCalculator cmc(&scannotner, &acc, postfix);
		if (cloneMetricRequired) {
			cmc.setOptionEachItem(optionEachItem);
			cmc.setOptionSummary(optionSummary);
			cmc.open(cloneMetricOutputFile);
		}

		metrics::LinebasedMetricsCalculator wmc(&scannotner, &acc, postfix);
		if (wordcountRequired) {
			wmc.setOptionEachItem(optionEachItem);
			wmc.setOptionSummary(optionSummary);
			wmc.open(wordcountOutputFile);
		}

		std:: vector<int> fileIDs;
		acc.getFiles(&fileIDs);

		int count = 0;

		std::string errorMessage;
		for (size_t i = 0; i < fileIDs.size(); ++i) {
			++count;
			int fileID = fileIDs[i];

			if (fileMetricRequired) {
				fmc.scannotFile(i);
			}

			if (cloneMetricRequired) {
				cmc.scannotFile(i);
			}

			if (wordcountRequired) {
				wmc.scannotFile(i);
			}
		}
		
		if (fileMetricRequired) {
			fmc.close();
		}

		if (cloneMetricRequired) {
			cmc.close();
		}

		if (wordcountRequired) {
			wmc.close();
		}

		return 0;
	}
};

#endif // METRICMAIN_H
