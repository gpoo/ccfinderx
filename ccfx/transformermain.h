#if ! defined TRANSFORMERMAIN_H
#define TRANSFORMERMAIN_H

#include <string>
#include <vector>
#include <ios>
#include <stdexcept>
#include "../common/hash_map_includer.h"
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/optional/optional.hpp>
#include <boost/pool/pool_alloc.hpp>

#include "rawclonepairdata.h"
#include "shapedfragmentcalculator.h"
#include "metricmain.h"
#include "../common/datastructureonfile.h"
#include "../threadqueue/threadqueue.h"

#pragma pack(push, 1)
struct id_and_stop {
	boost::uint64_t id;
	bool stop;
public:
	id_and_stop(const id_and_stop &right)
		: id(right.id), stop(right.stop)
	{
	}
	id_and_stop()
		: id(0), stop(false)
	{
	}
	id_and_stop(boost::uint64_t id_, bool stop_)
		: id(id_), stop(stop_)
	{
	}
};
#pragma pack(pop)

class TransformerMain {

private:
	std:: string outputFile;
	std:: string inputFile;
	bool optionVerbose;
	bool optionRecalculateTks;
	int shapingLevel; // =1 easy shaper, =2 soft shaper, =3 hard shaper
	onfile::Array<id_and_stop> cloneIDTransformTable;
	ThreadFunction threadFunction;

public:
	static int do_removingNonmaximalClonePairs(const std:: string &input, const std:: string &output, bool verbose)
	{
		NonmaximalPairRemover remover;
		rawclonepair::RawClonePairFileTransformer trans;
		if (! trans.filterFileByFile(output, input, &remover)) {
			std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
			return 1;
		}

		if (verbose) {
			std:: cerr << "> count of clone pairs removed by non-maximal clone pair filter: " << remover.getCountOfRemovedClonePairs() << std:: endl;
		}

		return 0;
	}

	static int do_id_transformation(const std:: string &input, const std:: string &output, bool verbose)
	{
		TransformerMain obj;
		obj.inputFile = input;
		obj.outputFile = output;
		obj.optionVerbose = verbose;
		obj.optionRecalculateTks = false;
		obj.shapingLevel = 0;

		{
			std:: string temp_file = ::make_temp_file_on_the_same_directory(obj.outputFile, "ccfxshaper1", ".tmp");
			if (! obj.cloneIDTransformTable.create(temp_file, true)) {
				std:: cerr << "error: can't create a temporary file (5)" << std:: endl;
				return 1;
			}
		}
		{
			std:: string tempFileForShapedFragments = ::make_temp_file_on_the_same_directory(output, "ccfxshaper2", ".tmp");

			DumShaper shaper(&obj);

			{
				rawclonepair::RawClonePairFileTransformer trans;
				if (! trans.filterFileByFile(tempFileForShapedFragments, input, &shaper)) {
					std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
					return 1;
				}
			}

			{
				IDTransformer idtransformer(&obj);
				rawclonepair::RawClonePairFileTransformer trans;
				if (! trans.filterFileByFile(output, tempFileForShapedFragments, &idtransformer)) {
					std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
					return 1;
				}
			}

			if (obj.optionVerbose) {
				std:: cerr << "> count of clone pairs removed by block shaper: " << shaper.getCountOfRemovedClonePairs() << std:: endl;
			}

			::remove(tempFileForShapedFragments.c_str());
		}

		std:: string temp_file = obj.cloneIDTransformTable.getFilePath();
		obj.cloneIDTransformTable.close();
		::remove(temp_file.c_str());

		return 0;
	}

	static int do_shaper(const std:: string &input, const std:: string &output, 
		int shaping_level, bool recalculate_tks, bool verbose)
	{
		TransformerMain obj;
		obj.inputFile = input;
		obj.outputFile = output;
		obj.optionVerbose = verbose;
		obj.optionRecalculateTks = recalculate_tks;
		obj.shapingLevel = shaping_level;

		if (obj.shapingLevel >= 2) {
			std:: string temp_file = ::make_temp_file_on_the_same_directory(obj.outputFile, "ccfxshaper1", ".tmp");
			if (! obj.cloneIDTransformTable.create(temp_file, true)) {
				std:: cerr << "error: can't create a temporary file (6)" << std:: endl;
				return 1;
			}
		}

		int r = obj.do_shaper();
		if (r != 0) {
			return r;
		}

		std:: string temp_file = obj.cloneIDTransformTable.getFilePath();
		obj.cloneIDTransformTable.close();
		::remove(temp_file.c_str());

		return r;
	}

	static int do_majoritarianShaper(const std:: string &input, const std:: string &output, bool verbose,
		size_t maxTrimming)
	{
		MajoritarianCalculator calculator;
		calculator.setMaxTrim(maxTrimming, maxTrimming);
		//calculator.setAppVersionChecker(APPVERSION[0], APPVERSION[1]);
		calculator.calc(input);

		Trimmer remover;
		remover.attachTrimmerTable(&calculator.refTrimmerTable());

		rawclonepair::RawClonePairFileTransformer trans;
		if (! trans.filterFileByFile(output, input, &remover)) {
			std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
			return 1;
		}

		if (verbose) {
			std:: cerr << "> count of clone pairs removed by majoritarian shaper: " << remover.getCountOfRemovedClonePairs() << std:: endl;
		}

		return 0;
	}

	TransformerMain()
		: optionVerbose(false), optionRecalculateTks(false), shapingLevel(2)
	{
	}

	int main(const std::vector<std::string> &argv) 
	{
		assert(argv.size() >= 2);
		if (argv.size() <= 2 || argv[2] == "-h" || argv[2] == "--help") {
			std::cout <<
				"Usage: ccfx T in.ccfxd OPTIONS -o out.ccfxd COMMAND" "\n"
				"  Transforms clone data." "\n"
				"Command" "\n"
				"  -s 2: applies soft shaper." "\n"
				"  -s 3: applies hard shaper." "\n"
				"Options" "\n"
				//"  -n dir: specify directory where preprocessed files are created." "\n"
				"  --threads=number: max working threads (0)." "\n"
				;
			return 0;
		}

		int shaping_level = 0;

		for (size_t i = 2; i < argv.size(); ++i) {
			std:: string argi = argv[i];
			if (boost::starts_with(argi, "-")) {
				if (argi == "-o") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -o requires an argument" << std:: endl;
						return 1;
					}
					outputFile = argv[i + 1];
					++i;
				}
				else if (argi == "-v") {
					optionVerbose = true;
				}
				else if (argi == "-s") {
					if (shaping_level != 0) {
						std:: cerr << "error: command -s specified twice" << std:: endl;
						return 1;
					}
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: command -n requres an argument" << std:: endl;
						return 1;
					}
					try {
						int value = boost::lexical_cast<int>(argv[i + 1]);
						if (! (2 <= value && value <= 3)) {
							std:: cerr << "error: range error for command -s" << std:: endl;
							return 1;
						}
						shaping_level = value;
					}
					catch(boost::bad_lexical_cast &) {
						std:: cerr << "error: invalid argument is given to command -s" << std:: endl;
						return 1;
					}
					++i;
				}
				//else if (argi == "-n") {
				//	if (i + 1 < argv.size()) {
				//		this->rawReader.addPreprocessFileDirectory(argv[i + 1]);
				//		++i;
				//	}
				//	else {
				//		std:: cerr << "error: command -s requres an argument" << std:: endl;
				//		return 1;
				//	}
				//}
				else {
					std::pair<int, std::string> r = threadFunction.scanOption(argi, (i + 1 < argv.size()) ? argv[i + 1] : std::string());
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
					force_extension(&inputFile, pairBinaryDiploidExtension, ".tmp");
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

		if (outputFile.empty()) {
			std:: cerr << "error: no output file is given" << std:: endl;
			return 1;
		}

		if (optionVerbose) {
			std::cerr << "> " << threadFunction.getVerboseMessage() << std::endl;
		}
		threadFunction.applyToSystem();

		if (shaping_level == 0) {
			std:: cerr << "error: no command is given" << std:: endl;
			return 1;
		}

		shapingLevel = shaping_level;
		if (shapingLevel >= 2) {
			std:: string temp_file = ::make_temp_file_on_the_same_directory(outputFile, "ccfxshaper1", ".tmp");
			if (! cloneIDTransformTable.create(temp_file, true)) {
				std:: cerr << "error: can't create a temporary file (7)" << std:: endl;
				return 1;
			}
		}

		optionRecalculateTks = true;

		int r = do_shaper();
		if (r != 0) {
			return r;
		}

		std:: string temp_file = cloneIDTransformTable.getFilePath();
		cloneIDTransformTable.close();
		::remove(temp_file.c_str());

		return r;
	}
private:
	rawclonepair::RawClonePairFileAccessor accessor;
	std:: string errorMessage;

#if defined USE_BOOST_POOL
	typedef std:: map<rawclonepair::RawFileBeginEnd, std::vector<boost::uint64_t>, 
		std::less<rawclonepair::RawFileBeginEnd>, 
		boost::fast_pool_allocator<std::pair<rawclonepair::RawFileBeginEnd, std::vector<boost::uint64_t> > > > EquivalentTable;
#else
	typedef std:: map<rawclonepair::RawFileBeginEnd, std::vector<boost::uint64_t> > EquivalentTable;
#endif

	class ShaperError : public std:: runtime_error {
	public:
		ShaperError() 
			: runtime_error("")
		{
		}
	};

	static bool less_wo_reference(const rawclonepair::RawClonePair &a, const rawclonepair::RawClonePair &b)
	{
		if (a.left.file < b.left.file) {
			return true;
		}
		else if (a.left.file == b.left.file) {
			if (a.right.file < b.right.file) {
				return true;
			}
			else if (a.right.file == b.right.file) {
				if (a.left.begin < b.left.begin) {
					return true;
				}
				else if (a.left.begin == b.left.begin) {
					if (a.left.end < b.left.end) {
						return true;
					}
					else if (a.left.end == b.left.end) {
						if (a.right.begin < b.right.begin) {
							return true;
						}
						else if (a.right.begin == b.right.begin) {
							if (a.right.end < b.right.end) {
								return true;
							}
							else if (a.right.end == b.right.end) {
								NULL;
							}
						}
					}
				}
			}
		}

		return false;
	}

	static bool equal_wo_reference(const rawclonepair::RawClonePair &a, const rawclonepair::RawClonePair &b)
	{
		return a.left.file == b.left.file && a.right.file == b.right.file
				&& a.left.begin == b.left.begin && a.left.end == b.left.end
				&& a.right.begin == b.right.begin && a.right.end == b.right.end;
	}

	static void build_id_transfom_table_from_equivs(
			std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *pIdTrans, 
			std::vector<std::vector<boost::uint64_t> > *pEquivs)
	{
		std::vector<std::vector<boost::uint64_t> > &equivs = *pEquivs;

		for (size_t i = 0; i < equivs.size(); ++i) {
			std::vector<boost::uint64_t> &equiv = equivs[i];
			std::sort(equiv.begin(), equiv.end());
			std::vector<boost::uint64_t>::iterator endPos = std::unique(equiv.begin(), equiv.end());
			equiv.erase(endPos, equiv.end());
		}
		
		std::vector<size_t> heads;
		heads.resize(equivs.size(), 0);

		while (true) {
			// remove empty equiv
			{
				size_t size = 0;
				size_t j = 0;
				while (true) {
					while (j < equivs.size() && equivs[j].empty()) {
						++j;
					}
					if (j == equivs.size()) { // not found
						break; // while true
					}
					if (j != size) {
						assert(equivs[size].empty());
						assert(! equivs[j].empty());
						equivs[size].swap(equivs[j]);
						std::swap(heads[size], heads[j]);
					}
					++j;
					++size;
				}
				equivs.resize(size);
				heads.resize(size);
			}

			// find the smallest head elemnt in equivs
			std::vector<size_t> smallests;
			bool allDone = true;
			for (size_t i = 0; i < heads.size(); ++i) {
				if (heads[i] < equivs[i].size()) {
					if (smallests.empty()) {
						smallests.push_back(i);
					}
					else {
						size_t smallest0 = smallests[0];
						boost::uint64_t ei = equivs[i][heads[i]];
						boost::uint64_t esi = equivs[smallest0][heads[smallest0]];
						if (ei < esi) {
							smallests.clear();
							smallests.push_back(i);
						}
						else if (ei == esi) {
							smallests.push_back(i);
						}
					}
					allDone = false;
				}
			}
			if (allDone) {
				break; // while true
			}

			// marge equivs that has same (the smallest head) element
			if (smallests.size() >= 2) {
				size_t smallest0 = smallests[0];
				for (size_t i = 1; i < smallests.size(); ++i) {
					size_t smallesti = smallests[i];
					//assert(equivs[smallesti][heads[smallesti]] == equivs[smallest0][heads[smallest0]]);
					equivs[smallest0].insert(equivs[smallest0].end(), equivs[smallesti].begin(), equivs[smallesti].end());
					equivs[smallesti].clear();
				}
				std::vector<boost::uint64_t> &eqs = equivs[smallest0];
				boost::uint64_t value = eqs[heads[smallest0]];
				std::sort(eqs.begin(), eqs.end());
				std::vector<boost::uint64_t>::iterator endPos = std::unique(eqs.begin(), eqs.end());
				equivs[smallest0].resize(endPos - eqs.begin());
				size_t j = heads[smallest0];
				while (eqs[j] != value) {
					++j;
					assert(j < eqs.size());
				}
				heads[smallest0] = j;
			}

			assert(smallests.size() >= 1);
			heads[smallests[0]] += 1;
		}
		
		std::vector<std::pair<boost::uint64_t, boost::uint64_t> > &idTrans = *pIdTrans;
		idTrans.clear();

		for (std::vector<std::vector<boost::uint64_t> >::const_iterator i = equivs.begin(); i != equivs.end(); ++i) {
			const std::vector<boost::uint64_t> &equiv = *i;
			for (size_t j = 0; j < equiv.size(); ++j) {
				assert(equiv[0] <= equiv[j]);
				idTrans.push_back(std::pair<boost::uint64_t, boost::uint64_t>(equiv[j], equiv[0]));
			}
		}
		std::sort(idTrans.begin(), idTrans.end());
	}

	friend class DumShaper;
	class DumShaper : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		TransformerMain &base;
		long long countOfRemovedClonePairs;
	public:		
		DumShaper(TransformerMain *pBase)
			: base(*pBase), countOfRemovedClonePairs(0)
		{
		}
		bool isValidFileID(int fileID)
		{
			return true; // will not do filtering by fileID
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			return true; // will not do filtering by cloneID
		}
		void transformPairs(std:: vector<rawclonepair::RawClonePair> *pPairs)
		{
			std:: vector<rawclonepair::RawClonePair> &pairs = *pPairs;

			if (pairs.empty()) {
				return;
			}

			const rawclonepair::RawClonePair &p0 = pairs[0];
			boost::int32_t leftFileID = p0.left.file;

			// determin the smallest clone id for each fragment
			std::vector<std::pair<boost::uint64_t, boost::uint64_t> > idTrans;
			{
				EquivalentTable equivalents;
				for (size_t i = 0; i < pairs.size(); ++i) {
					const rawclonepair::RawClonePair &pair = pairs[i];
					if (pair.left.end - pair.left.begin > 0) {
						equivalents[pair.left].push_back(pair.reference);
					}
				}

				std::vector<std::vector<boost::uint64_t> > equivs;
				equivs.reserve(equivalents.size());
				for (EquivalentTable::iterator i = equivalents.begin(); i != equivalents.end(); ++i) {
					equivs.resize(equivs.size() + 1);
					equivs.back().swap(i->second);
				}

				build_id_transfom_table_from_equivs(&idTrans, &equivs);
			}

			if (! idTrans.empty()) {
				const std::pair<boost::uint64_t, boost::uint64_t> &idTransLast = idTrans.back();
				if (! (idTransLast.first < base.cloneIDTransformTable.size())) {
					unsigned long long curSize = base.cloneIDTransformTable.size();
					std::vector<id_and_stop> newItems;
					newItems.reserve(idTransLast.first + 1 - curSize);
					for (unsigned long long i = curSize; i < idTransLast.first + 1; ++i) {
						newItems.push_back(id_and_stop(i, true));
					}
					base.cloneIDTransformTable.extend(&newItems[0], newItems.size());
					assert(base.cloneIDTransformTable.size() == idTransLast.first + 1);
				}
			}
			for (size_t i = 0; i < idTrans.size(); ++i) {
				const std::pair<boost::uint64_t, boost::uint64_t> &idTransI = idTrans[i];
				boost::uint64_t fromID = idTransI.first;
				boost::uint64_t toID = idTransI.second;
				assert(toID <= fromID);
				id_and_stop r;
				base.cloneIDTransformTable.get(&r, fromID);
				assert(r.id <= fromID);
				if (toID < r.id) {
					base.cloneIDTransformTable.set(fromID, id_and_stop(toID, false));
					base.cloneIDTransformTable.set(r.id, id_and_stop(toID, false));
				}
				else if (toID > r.id) {
					base.cloneIDTransformTable.set(toID, id_and_stop(r.id, false));
				}
			}

			std:: sort(pairs.begin(), pairs.end(), less_wo_reference);
			std:: vector<rawclonepair::RawClonePair>::iterator endPos = std::unique(pairs.begin(), pairs.end(), equal_wo_reference);
			countOfRemovedClonePairs += pairs.end() - endPos;
			pairs.resize(endPos - pairs.begin());
		}
	public:
		long long getCountOfRemovedClonePairs() const
		{
			return countOfRemovedClonePairs;
		}
	};

	friend class Shaper;
	class Shaper : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		TransformerMain &base;
		size_t tksValue;
		long long countOfRemovedClonePairs;
		PreprocessedFileReader scannotner;
		ThreadQueue<std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *> *pQueIdTrans;
		boost::thread *pEaterIdTrans;

	public:
		virtual ~Shaper()
		{
			join();
			delete pQueIdTrans;
		}
		Shaper(TransformerMain *pBase)
			: base(*pBase), tksValue(0), countOfRemovedClonePairs(0), pQueIdTrans(NULL), 
			pEaterIdTrans(NULL)
		{
			pQueIdTrans = new ThreadQueue<std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *>(10);
			pEaterIdTrans = new boost::thread(boost::bind(&Shaper::idtrans_reflect_to_base, this, pQueIdTrans));
		}
	public:
		void join()
		{
			if (pEaterIdTrans != NULL) {
				(*pQueIdTrans).push(NULL);
				(*pEaterIdTrans).join();
				// thread object deletion at the below seems cause error at runtime. why?
				//delete pEaterIdTrans;
			}
		}
	private:
		void idtrans_reflect_to_base(ThreadQueue<std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *> *pQueIdTrans) 
		{
			std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *pIdTrans;
			while ((pIdTrans = (*pQueIdTrans).pop()) != NULL) {
				std::vector<std::pair<boost::uint64_t, boost::uint64_t> > &idTrans = *pIdTrans;
				if (! idTrans.empty()) {
					const std::pair<boost::uint64_t, boost::uint64_t> &idTransLast = idTrans.back();
					if (! (idTransLast.first < base.cloneIDTransformTable.size())) {
						unsigned long long curSize = base.cloneIDTransformTable.size();
						std::vector<id_and_stop> newItems;
						newItems.reserve(idTransLast.first + 1 - curSize);
						for (unsigned long long i = curSize; i < idTransLast.first + 1; ++i) {
							newItems.push_back(id_and_stop(i, true));
						}
						base.cloneIDTransformTable.extend(&newItems[0], newItems.size());
						assert(base.cloneIDTransformTable.size() == idTransLast.first + 1);
					}
				}
				for (size_t i = 0; i < idTrans.size(); ++i) {
					const std::pair<boost::uint64_t, boost::uint64_t> &idTransI = idTrans[i];
					boost::uint64_t fromID = idTransI.first;
					boost::uint64_t toID = idTransI.second;
					assert(toID <= fromID);
					id_and_stop r;
					base.cloneIDTransformTable.get(&r, fromID);
					assert(r.id <= fromID);
					if (toID < r.id) {
						base.cloneIDTransformTable.set(fromID, id_and_stop(toID, false));
						base.cloneIDTransformTable.set(r.id, id_and_stop(toID, false));
					}
					else if (toID > r.id) {
						base.cloneIDTransformTable.set(toID, id_and_stop(r.id, false));
					}
				}
				delete pIdTrans;
			}
		}
	public:
		void setRawReader(const PreprocessedFileRawReader &rawReader_)
		{
			scannotner.setRawReader(rawReader_);
		}
		bool isValidFileID(int fileID)
		{
			return true; // will not do filtering by fileID
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			return true; // will not do filtering by cloneID
		}
		void transformPairs(std:: vector<rawclonepair::RawClonePair> *pPairs)
		{
			assert(2 <= base.shapingLevel && base.shapingLevel <= 3);

			const std:: vector<rawclonepair::RawClonePair> &pairs = *pPairs;

			if (pairs.empty()) {
				return;
			}

			const rawclonepair::RawClonePair &p0 = pairs[0];
			boost::int32_t leftFileID = p0.left.file;

			std:: string fileName;
			size_t fileLength;
			base.accessor.getFileDescription(leftFileID, &fileName, &fileLength);
			fileName = INNER2SYS(fileName);

			std::vector<std::string> p = base.accessor.getOptionValues(PREPROCESSED_FILE_POSTFIX);
			std::string postfix = (! p.empty()) ? p.back() : ("." + base.accessor.getPreprocessScript() + ".ccfxprep");
			std:: vector<ccfx_token_t> seq;
			if (! getPreprocessedSequenceOfFile(&seq, fileName, postfix, &scannotner, &base.errorMessage)) {
				throw ShaperError();
			}
			assert(seq.size() == fileLength + 1);

			shaper::ShapedFragmentsCalculator<ccfx_token_t> shaper;
			shaper.setParens(scannotner.refParens());
			shaper.setPrefixes(scannotner.refPrefixes());
			shaper.setSuffixes(scannotner.refSuffixes());
			
			//std:: cout << std:: endl;		
			//for (size_t i = 0; i < pairs.size(); ++i) {
			//	const rawclonepair::RawClonePair &pair = pairs[i];
			//	std:: cout << pair.left.file << "." << pair.left.begin << "-" << pair.left.end << ", " << pair.right.file << "." << pair.right.begin << "-" << pair.right.end << ", " << pair.reference << std:: endl;
			//}

			const int shift_by_first_zero = 1;

			size_t minimumLength = 0;
			std::vector<std::string> minLenValue = base.accessor.getOptionValues("b");
			if (! minLenValue.empty()) {
				try {
					minimumLength = boost::lexical_cast<int, std::string>(minLenValue.back());	
				}
				catch(boost::bad_lexical_cast &) {
					// do nothing
				}
			}

			// calc a set of shaped fragments from the left-side of the code fragments of clone pairs
			std:: vector<rawclonepair::RawFileBeginEnd> shapedLeftFragments;
			shapedLeftFragments.resize(pairs.size());
#pragma omp parallel for
			for (int i = 0; i < pairs.size(); ++i) {
				const rawclonepair::RawClonePair &pair = pairs[i];
				if (i > 0 && pair.left == pairs[i - 1].left) {
					// do nothing
				}
				else {
					rawclonepair::RawFileBeginEnd &f = shapedLeftFragments[i];
					f = to_shaped_fragment(pair.left, seq, &shaper);
				}
			}

			// determin the smallest clone id for each shaped fragment
			std::vector<std::pair<boost::uint64_t, boost::uint64_t> > *pIdTrans = new std::vector<std::pair<boost::uint64_t, boost::uint64_t> >();
			std::vector<std::pair<boost::uint64_t, boost::uint64_t> > &idTrans = *pIdTrans;
			{
				EquivalentTable equivalents;
				for (size_t i = 0; i < pairs.size(); ++i) {
					const rawclonepair::RawClonePair &pair = pairs[i];
					if (i > 0 && pair.left == pairs[i - 1].left) {
						shapedLeftFragments[i] = shapedLeftFragments[i - 1];
					}
					const rawclonepair::RawFileBeginEnd &f = shapedLeftFragments[i];
					if (f.end - f.begin > 0) {
						std::vector<boost::uint64_t> &e = equivalents[f];
						if (e.empty() || e.back() != pair.reference) {
							e.push_back(pair.reference);
						}
					}
				}
				
				std::vector<std::vector<boost::uint64_t> > equivs;
				equivs.reserve(equivalents.size());
				for (EquivalentTable::iterator i = equivalents.begin(); i != equivalents.end(); ++i) {
					equivs.resize(equivs.size() + 1);
					equivs.back().swap(i->second);
				}

				build_id_transfom_table_from_equivs(&idTrans, &equivs);
			}

			std:: vector<rawclonepair::RawClonePair> shapedPairs;
			{
				// make shaped pair, that is, determin left and right code fragments and add the clone-set id of the shortest original code fragment
				shapedPairs.reserve(pairs.size());
				for (size_t i = 0; i < pairs.size(); ++i) {
					const rawclonepair::RawFileBeginEnd &f = shapedLeftFragments[i];
					if (f.end - f.begin > 0) {
						const rawclonepair::RawClonePair &pair = pairs[i];
						shapedPairs.resize(shapedPairs.size() + 1);
						rawclonepair::RawClonePair &p = shapedPairs.back();
						
						p.left = f;
						p.right = pair.right;
						p.right.begin += f.begin - pair.left.begin;
						p.right.end -= pair.left.end - f.end;

						{
							std::vector<std::pair<boost::uint64_t, boost::uint64_t> >::const_iterator chk = std::lower_bound(idTrans.begin(), idTrans.end(), std::pair<boost::uint64_t, boost::uint64_t>(pair.reference, 0));
							assert(chk != idTrans.end());
							assert(chk->first == pair.reference);
						}
						p.reference = pair.reference;
					}
					else {
						++countOfRemovedClonePairs;
					}
				}
			}
			(*pQueIdTrans).push(pIdTrans);

			std:: sort(shapedPairs.begin(), shapedPairs.end(), less_wo_reference);
			std:: vector<rawclonepair::RawClonePair>::iterator endPos = std::unique(shapedPairs.begin(), shapedPairs.end(), equal_wo_reference);
			countOfRemovedClonePairs += shapedPairs.end() - endPos;
			shapedPairs.erase(endPos, shapedPairs.end());

			(*pPairs).swap(shapedPairs);
		}
		void filterOptions(std::vector<std::pair<std::string/* name */, std::string/* value */> > *pOptions)
		{
			std::vector<std::pair<std::string/* name */, std::string/* value */> > &table = *pOptions;
			for (size_t i = 0; i < table.size(); ++i) {
				const std::string &name = table[i].first;
				std::string &value = table[i].second;
				if (name == "s") {
					value = (boost::format("%d") % base.shapingLevel).str();
				}
				else if (name == "t") {
					try {
						tksValue = boost::lexical_cast<int, std::string>(value);
					}
					catch(boost::bad_lexical_cast &) {
						// do nothing
					}
				}
			}
		}
	public:
		long long getCountOfRemovedClonePairs() const
		{
			return countOfRemovedClonePairs;
		}
	private:
		rawclonepair::RawFileBeginEnd to_shaped_fragment(const rawclonepair::RawFileBeginEnd &leftCode, 
				const std:: vector<ccfx_token_t> &seq, shaper::ShapedFragmentsCalculator<ccfx_token_t> *pShaper)
		{
			const int shift_by_first_zero = 1;

			size_t minimumLength = 0;
			std::vector<std::string> minLenValue = base.accessor.getOptionValues("b");
			if (! minLenValue.empty()) {
				try {
					minimumLength = boost::lexical_cast<int, std::string>(minLenValue.back());	
				}
				catch(boost::bad_lexical_cast &) {
					// do nothing
				}
			}

			shaper::ShapedFragmentsCalculator<ccfx_token_t> &shaper = *pShaper;

			rawclonepair::RawFileBeginEnd shapedLeft = leftCode;
			shapedLeft.end = shapedLeft.begin; // make it zero length

			// extract shaped code fragments from the left code fragment
			std:: vector<shaper::ShapedFragmentPosition> fragmentsLargerThanThreshold;
			{
				std:: vector<shaper::ShapedFragmentPosition> fragments;
				assert(leftCode.end + shift_by_first_zero <= seq.size());
				shaper.calc(&fragments, seq, leftCode.begin + shift_by_first_zero, leftCode.end + shift_by_first_zero, 
						base.shapingLevel == 2 ? shaper::HAT_FRAGMENT : shaper::CAP_FRAGMENT);
				
				fragmentsLargerThanThreshold.reserve(fragments.size());
				for (size_t j = 0; j < fragments.size(); ++j) {
					const shaper::ShapedFragmentPosition &p = fragments[j];
					if (p.end - p.begin >= minimumLength) {
						fragmentsLargerThanThreshold.push_back(p);
					}
				}
			}

			// merge the shaped code fragments included the left code fragment, into a fragment
			if (! fragmentsLargerThanThreshold.empty()) {
				const shaper::ShapedFragmentPosition &p0 = fragmentsLargerThanThreshold[0];
				size_t minBegin = p0.begin;
				size_t maxEnd = p0.end;
				for (size_t j = 1; j < fragmentsLargerThanThreshold.size(); ++j) {
					const shaper::ShapedFragmentPosition &p = fragmentsLargerThanThreshold[j];
					if (p.begin < minBegin) {
						minBegin = p.begin;
					}
					if (p.end > maxEnd) {
						maxEnd = p.end;
					}
				}
				assert(shift_by_first_zero <= minBegin && minBegin <= maxEnd);
				shapedLeft.begin = minBegin - shift_by_first_zero;
				shapedLeft.end = maxEnd - shift_by_first_zero;
				assert(shapedLeft.file == leftCode.file);
				assert(leftCode.begin <= shapedLeft.begin && shapedLeft.end <= leftCode.end);
			}

			if (base.optionRecalculateTks && tksValue >= 1) {
				size_t tks = metrics::calcTKS(seq, shapedLeft.begin + shift_by_first_zero, shapedLeft.end + shift_by_first_zero);
				if (tks < tksValue) {
					shapedLeft.end = shapedLeft.begin; // make it zero length
				}
			}

			return shapedLeft;
		}
	};
	
	friend class IDTransformer;
	class IDTransformer : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		TransformerMain &base;
	public:
		IDTransformer(TransformerMain *pBase)
			: base(*pBase)
		{
		}
		bool isValidFileID(int fileID)
		{
			return true; // will not do filtering by fileID
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			return true; // will not do filtering by cloneID
		}
		void transformPairs(std:: vector<rawclonepair::RawClonePair> *pPairs)
		{
			std:: vector<rawclonepair::RawClonePair> &pairs = *pPairs;

			HASH_MAP<boost::uint64_t, boost::uint64_t> idTransCache;
			for (size_t i = 0; i < pairs.size(); ++i) {
				rawclonepair::RawClonePair &pair = pairs[i];

				HASH_MAP<boost::uint64_t, boost::uint64_t>::iterator j = idTransCache.find(pair.reference);
				if (j == idTransCache.end()) {
					if (pair.reference < base.cloneIDTransformTable.size()) {
						boost::uint64_t stopID = getStopID(pair.reference);
						idTransCache[pair.reference] = stopID;
						pair.reference = stopID;
					}
					else {
						idTransCache[pair.reference] = pair.reference;
						//pair.reference = pair.reference;
					}
				}
				else {
					pair.reference = j->second;
				}
			}
		}
	private:
		boost::uint64_t getStopID(boost::uint64_t id)
		{
			id_and_stop r;
			base.cloneIDTransformTable.get(&r, id);
			assert(r.id <= id);
			if (r.stop || r.id == id) {
				return r.id;
			}
			else {
				boost::uint64_t stopID = getStopID(r.id);
				base.cloneIDTransformTable.set(id, id_and_stop(stopID, true));
				return stopID;
			}
		}
	};
	
	class NonmaximalPairRemover : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		long long countOfRemovedClonePairs;
	public:
		NonmaximalPairRemover()
			: countOfRemovedClonePairs(0)
		{
		}
	public:
		long long getCountOfRemovedClonePairs() const
		{
			return countOfRemovedClonePairs;
		}
	public:
		bool isValidFileID(int fileID)
		{
			return true; // will not do filtering by fileID
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			return true; // will not do filtering by cloneID
		}
		void transformPairs(std:: vector<rawclonepair::RawClonePair> *pPairs)
		{
			std:: vector<rawclonepair::RawClonePair> &pairs = *pPairs; // must be sorted
			
			boost::dynamic_bitset<> nonmaximal;
			nonmaximal.resize(pairs.size(), false);

			size_t i = 0; 
			while (i < pairs.size()) {
				const rawclonepair::RawClonePair &pairI = pairs[i];
				size_t j = i + 1;
				while (j < pairs.size() && pairs[j].right.file == pairI.right.file) {
					assert(pairs[j].left.file == pairI.left.file);
					++j;
				}

				{
					size_t begin = i;
					size_t end = j;
					
					for (size_t k = begin; k < end; ++k) {
						if (nonmaximal.test(k)) {
							continue; // k
						}
						const rawclonepair::RawClonePair &pairK = pairs[k];
						for (size_t m = k + 1; m < end; ++m) {
							if (nonmaximal.test(m)) {
								continue; // m
							}
							const rawclonepair::RawClonePair &pairM = pairs[m];
							if (pairM.left.end - pairM.left.begin < pairK.left.end - pairK.left.begin) {
								if (pairK.left.begin <= pairM.left.begin && pairM.left.end <= pairK.left.end
										&& pairK.right.begin <= pairM.right.begin && pairM.right.end <= pairK.right.end) {
									nonmaximal.set(m, true);
								}
							}
							else {
								if (pairM.left.begin <= pairK.left.begin && pairK.left.end <= pairM.left.end
										&& pairM.right.begin <= pairK.right.begin && pairK.right.end <= pairM.right.end) {
									nonmaximal.set(k, true);
								}
							}
						}
					}
				}

				i = j;
			}
			std:: vector<rawclonepair::RawClonePair> temp;
			temp.reserve(nonmaximal.size() - nonmaximal.count());
			for (i = 0; i < pairs.size(); ++i) {
				if (! nonmaximal.test(i)) {
					temp.push_back(pairs[i]);
				}
			}
			pairs.swap(temp);
			countOfRemovedClonePairs += nonmaximal.count();
		}
	};

	int do_shaper()
	{
		if (optionVerbose) {
			switch (shapingLevel) {
			case 2:
				std:: cerr << "> applying soft block shaper" << std:: endl;
				break;
			case 3:
				std:: cerr << "> applying hard block shaper" << std:: endl;
				break;
			default:
				assert(false);
				break;
			}
		}

		if (! accessor.open(inputFile, rawclonepair::RawClonePairFileAccessor::FILEDATA)) {
			std:: cerr << "error: " << accessor.getErrorMessage() << std:: endl;
			return 1;
		}

		boost::int32_t curShapingLevel = -1;
		std::vector<std::string> sl = accessor.getOptionValues("s");
		if (! sl.empty()) {
			curShapingLevel = boost::lexical_cast<int, std::string>(sl.back());	
		}
		if (shapingLevel < curShapingLevel) {
			std:: cerr << "error: wrong level for block shaper" << std:: endl;
			return 1;
		}

		std::vector<std::string> prepDirs = accessor.getOptionValues("n");
		for (std::vector<std::string>::iterator pi = prepDirs.begin(); pi != prepDirs.end(); ++pi) {
			*pi = INNER2SYS(*pi);
		}
		PreprocessedFileRawReader rawReader;
		rawReader.setPreprocessFileDirectories(prepDirs);

		std:: string tempFileForShapedFragments = ::make_temp_file_on_the_same_directory(outputFile, "ccfxshaper2", ".tmp");

		boost::int64_t removedPairCount = 0;
		Shaper shaper(this); // debug, moved out from the below block
		{
			shaper.setRawReader(rawReader);

			try {
				rawclonepair::RawClonePairFileTransformer trans;
				if (! trans.filterFileByFile(tempFileForShapedFragments, inputFile, &shaper)) {
					std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
					return 1;
				}
			}
			catch (ShaperError &) {
				std:: cerr << "error: " << errorMessage << std:: endl;
				return 1;
			}

			removedPairCount = shaper.getCountOfRemovedClonePairs();

			shaper.join(); // this thread uses the data generated by the sub-thread that has been forked by the "shaper"
		}

		{
			IDTransformer idtransformer(this);
			rawclonepair::RawClonePairFileTransformer trans;
			if (! trans.filterFileByFile(outputFile, tempFileForShapedFragments, &idtransformer)) {
				std:: cerr << "error: " << trans.getErrorMessage() << std:: endl;
				return 1;
			}
		}

		if (optionVerbose) {
			std:: cerr << "> count of clone pairs removed by block shaper: " << removedPairCount << std:: endl;
		}

		::remove(tempFileForShapedFragments.c_str());

		return 0;
	}

	struct TrimDown {
	public:
		boost::uint64_t targetID;
		std::pair<size_t, size_t> trimming;
	public:
		TrimDown()
			: targetID(0), trimming(0, 0)
		{
		}
		TrimDown(const TrimDown &right)
			: targetID(right.targetID), trimming(right.trimming)
		{
		}
		TrimDown(boost::uint64_t targetID_, size_t headTrimming, size_t tailTrimming)
			: targetID(targetID_), trimming(headTrimming, tailTrimming)
		{
		}
	public:
		inline size_t length() const
		{
			return trimming.first + trimming.second;
		}
		bool operator<(const TrimDown &right) const
		{
			if (targetID < right.targetID) {
				return true;
			}
			else if (targetID == right.targetID) {
				if (trimming < right.trimming) {
					return true;
				}
				else if (trimming == right.trimming) {
				}
			}
			return false;
		}
		bool operator==(const TrimDown &right) const
		{
			return targetID == right.targetID && trimming == right.trimming;
		}
		bool isTruelyIncludedBy(const TrimDown &right) const
		{
			return trimming.first <= right.trimming.first && trimming.second <= right.trimming.second 
					&& trimming != right.trimming;
		}
	};
	class MajoritarianCalculator {
	private:
		HASH_MAP<boost::uint64_t, TrimDown> trimmerTable;
		std::pair<size_t, size_t> trimmingMaxes;

	public:
		MajoritarianCalculator()
			: trimmerTable(), trimmingMaxes(0, 0)
		{
		}

	public:
		void setMaxTrim(size_t headTrimmingMax, size_t tailTrimmingMax)
		{
			trimmingMaxes.first = headTrimmingMax;
			trimmingMaxes.second = tailTrimmingMax;
		}
		const HASH_MAP<boost::uint64_t, TrimDown> &refTrimmerTable() const
		{
			return trimmerTable;
		}
		void calc(const std::string &input)
		{
			trimmerTable.clear();

			HASH_MAP<boost::uint64_t, std::vector<TrimDown> > trimDownTable;
			rawclonepair::RawClonePairFileAccessor accessor;
			accessor.open(input,
					rawclonepair::RawClonePairFileAccessor::FILEDATA
					| rawclonepair::RawClonePairFileAccessor::CLONEDATA);
			std::vector<int> fileIDs;
			accessor.getFiles(&fileIDs);
			if (fileIDs.size() > 0) {
				size_t fiPrefetched = 0;
				int fileIDPrefetched = fileIDs[fiPrefetched];
				std::vector<rawclonepair::RawClonePair> clonePairsPrefetched;
				accessor.getRawClonePairsOfFile(fileIDPrefetched, &clonePairsPrefetched);
				for (size_t fi = 0; fi < fileIDs.size(); ++fi) {
					int fileID = fileIDs[fi];
					assert(fi == fiPrefetched);
					std::vector<rawclonepair::RawClonePair> clonePairs;
					clonePairs.swap(clonePairsPrefetched);
#pragma omp parallel sections
					{
#pragma omp section
						{
							accumTrimDownTable(clonePairs, &trimDownTable);
						}
#pragma omp section
						{
							if (++fiPrefetched < fileIDs.size()) {
								fileIDPrefetched = fileIDs[fiPrefetched];
								accessor.getRawClonePairsOfFile(fileIDPrefetched, &clonePairsPrefetched);
							}
						}
					} // end #pragma omp sections
				}
			}

			for (HASH_MAP<boost::uint64_t, std::vector<TrimDown> >::iterator i = trimDownTable.begin(); i != trimDownTable.end(); ++i) {
				boost::uint64_t src = i->first;
				std::vector<TrimDown> &ts = i->second;
				//for (size_t j = 0; j < ts.size(); ++j) {
				//	const TrimDown &t = ts[j];
				//	std::cout << (boost::format("targetid = %d, head = %d, tail = %d") % (int)t.targetID % (int)t.trimming.first % (int) t.trimming.second) << std::endl;
				//}
				if (ts.size() == 1) {
					TrimDown composed = ts[0];
					HASH_MAP<boost::uint64_t, std::vector<TrimDown> >::const_iterator j;
					while ((j = trimDownTable.find(composed.targetID)) != trimDownTable.end() && j->second.size() == 1) {
						const TrimDown t = j->second[0];
						composed.targetID = t.targetID;
						composed.trimming.first += t.trimming.first;
						composed.trimming.second += t.trimming.second;
					}
					ts[0] = composed;
				}
			}

			for (HASH_MAP<boost::uint64_t, std::vector<TrimDown> >::const_iterator i = trimDownTable.begin(); i != trimDownTable.end(); ++i) {
				boost::uint64_t src = i->first;
				const std::vector<TrimDown> &ts = i->second;
				if (ts.size() == 1) {
					trimmerTable[src] = ts[0];
				}
			}
		}

	private:
		void accumTrimDownTable(const std::vector<rawclonepair::RawClonePair> &pairs, 
				HASH_MAP<boost::uint64_t, std::vector<TrimDown> > *pTrimDownTable)
		{
			HASH_MAP<boost::uint64_t, std::vector<TrimDown> > &trimDownTable = *pTrimDownTable;
			for (size_t i = 0; i < pairs.size(); ++i) {
				const rawclonepair::RawClonePair &pairI = pairs[i];
				for (size_t j = i + 1; j < pairs.size(); ++j) {
					const rawclonepair::RawClonePair &pairJ = pairs[j];
					if (pairI.left == pairJ.left) {
						// do nothing
					}
					else if (pairI.left.begin <= pairJ.left.begin && pairJ.left.end <= pairI.left.end && pairJ.reference != pairI.reference) {
						assert(pairJ.left.file == pairI.left.file);
						size_t ht = pairJ.left.begin - pairI.left.begin;
						size_t tt = pairI.left.end - pairJ.left.end;
						if (ht < trimmingMaxes.first && tt < trimmingMaxes.second) {
							std::vector<TrimDown> &trimDowns = trimDownTable[pairI.reference];
							add_and_remove(&trimDowns, TrimDown(pairJ.reference, ht, tt));
						}
					}
					else if (pairJ.left.begin <= pairI.left.begin && pairI.left.end <= pairJ.left.end && pairJ.reference != pairI.reference) {
						assert(pairJ.left.file == pairI.left.file);
						size_t ht = pairI.left.begin - pairJ.left.begin;
						size_t tt = pairJ.left.end - pairI.left.end;
						if (ht < trimmingMaxes.first && tt < trimmingMaxes.second) {
							std::vector<TrimDown> &trimDowns = trimDownTable[pairJ.reference];
							add_and_remove(&trimDowns, TrimDown(pairI.reference, ht, tt));
						}
					}
				}
			}
		}
		static void add_and_remove(std::vector<TrimDown> *pTrimDowns, const TrimDown &newOne)
		{
			std::vector<TrimDown> &trimDowns = *pTrimDowns;
			bool newOneReplaceSomething = false;
			for (std::vector<TrimDown>::const_iterator i = trimDowns.begin(); i != trimDowns.end(); ++i) {
				if (newOne == *i || newOne.isTruelyIncludedBy(*i)) {
					return; // newOne is included by one of (*pTrimDowns), so do not add newOne to the set.
				}
				if ((*i).isTruelyIncludedBy(newOne)) {
					newOneReplaceSomething = true;
				}
			}

			trimDowns.push_back(newOne);

			if (newOneReplaceSomething) {
				std::vector<TrimDown> r;
				for (std::vector<TrimDown>::const_iterator i = trimDowns.begin(); i != trimDowns.end(); ++i) {
					if (! (*i).isTruelyIncludedBy(newOne)) {
						r.push_back(*i);
					}
				}
				trimDowns.swap(r);
			}
		}
	};
	class Trimmer : public rawclonepair::RawClonePairFileTransformer::FilterFileByFile
	{
	private:
		long long countOfRemovedClonePairs;
		const HASH_MAP<boost::uint64_t, TrimDown> *pTrimmerTable;

	public:
		Trimmer()
			: countOfRemovedClonePairs(0)
		{
		}
	public:
		long long getCountOfRemovedClonePairs() const
		{
			return countOfRemovedClonePairs;
		}
		void attachTrimmerTable(const HASH_MAP<boost::uint64_t, TrimDown> *pTrimmerTable_)
		{
			pTrimmerTable = pTrimmerTable_;
		}
	public:
		bool isValidFileID(int fileID)
		{
			return true; // will not do filtering by fileID
		}
		bool isValidCloneID(boost::uint64_t cloneID) 
		{
			return true; // will not do filtering by cloneID
		}
		void transformPairs(std:: vector<rawclonepair::RawClonePair> *pPairs)
		{
			std:: vector<rawclonepair::RawClonePair> &pairs = *pPairs; // must be sorted
			const HASH_MAP<boost::uint64_t, TrimDown> &trimmerTable = *pTrimmerTable;

#pragma omp parallel for
			for (int i = 0; i < pairs.size(); ++i) {
				rawclonepair::RawClonePair &pair = pairs[i];
				HASH_MAP<boost::uint64_t, TrimDown>::const_iterator j = trimmerTable.find(pair.reference);
				if (j != trimmerTable.end()) {
					const TrimDown &td = j->second;
					assert(pair.left.end - pair.left.begin >= td.length());
					assert(pair.right.end - pair.right.begin >= td.length());
					pair.left.begin += td.trimming.first;
					pair.left.end -= td.trimming.second;
					pair.right.begin += td.trimming.first;
					pair.right.end -= td.trimming.second;
					pair.reference = td.targetID;
				}
			}
			size_t preSize = pairs.size();
			std::sort(pairs.begin(), pairs.end());
			std:: vector<rawclonepair::RawClonePair>::iterator endi = std::unique(pairs.begin(), pairs.end());
			pairs.resize(std::distance(pairs.begin(), endi));
			size_t postSize = pairs.size();

			countOfRemovedClonePairs += preSize - postSize;
		}
	};
};

#endif // TRANSFORMERMAIN_H

