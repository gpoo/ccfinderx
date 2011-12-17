#if ! defined PREPROCESSORINVOKER_H
#define PREPROCESSORINVOKER_H

#include <string>
#include <iostream>
#include <map>
#include <vector>

#include "../common/unportable.h"
#include "../common/filestructwrapper.h"
#include "../common/argvbuilder.h"

#if defined _MSC_VER
#include <process.h> // spawn
#endif

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include "../common/unportable.h"
#include "../common/utf8support.h"

#include "ccfxcommon.h"

class Param {
public:
	enum {
		ANY_MATCH = 1,
		EXACT_MATCH = 2,
		P_MATCH = 3
	};
public:
	static std::string toName(int paramOperation)
	{
		switch (paramOperation) {
		case ANY_MATCH:
			return "ANY_MATCH";
			break;
		case EXACT_MATCH:
			return "EXACT_MATCH";
			break;
		case P_MATCH:
			return "P_MATCH"; 
			break;
		default:
			assert(false);
		}
		return "dummy"; // never reach this sentence!
	}
};

class preprocessor_invoker
{
private:
	std::string argv0;
	std::string preprocessScriptName;
	PathTime preprocessScript;
	bool optionVerbose;
	std::string optionEncoding;
	bool scriptTableReadingDone;
	std::string tempFileDir;
	std::string postfix;
	std::vector<std::string> optionStrings;
	int optionMaxWorkerThreads;
	std::map<std::string/* scriptName */, std::set<std::string> /* extensions */> preprocessorExtensionTable;
	std:: map<std:: string/* preprocessor */, PathTime> preprocessorTable;
	std::vector<std::string> prepDirs;
	std::vector<std::string> extraPrepScriptFiles;

public:
	preprocessor_invoker()
	: optionVerbose(false), optionEncoding("char"), scriptTableReadingDone(false), optionMaxWorkerThreads(0)
	{
	}
	void setOptionVerbose(bool optionVerbose_)
	{
		optionVerbose = optionVerbose_;
	}
	void setEncodng(const std::string &encoding)
	{
		optionEncoding = encoding;
	}
	bool setPreprocessorName(const std::string &preprocessScriptName_)
	{
		assert(! preprocessScriptName_.empty());
		assert(scriptTableReadingDone);

		std:: map<std:: string, PathTime>::const_iterator si = preprocessorTable.find(preprocessScriptName_);

		if (si == preprocessorTable.end()) {
			return false; // fail "error: the named preprocess script not found"
		}
		
		preprocessScriptName = preprocessScriptName_;
		preprocessScript = si->second;
		assert(! preprocessScriptName.empty());
		
		postfix.clear();

		return true;
	}
	boost::optional<std::string/* error message */ > setPreprocessorOptions(const std::vector<std::string> &optionStrings_)
	{
		assert(! preprocessScriptName.empty());
		assert(scriptTableReadingDone);

		// normalize preprocessor option string
		//std::cerr << "debug: tempFileDir = " << tempFileDir << std::endl;
		std::string tempFile1 = ::make_temp_file_on_the_same_directory(tempFileDir, "ccfx-prep", ".tmp");

		std::string scriptPath = ::make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);

		ArgvBuilder argv;
		argv.push_back(thePythonInterpreterPath);
		argv.push_back(scriptPath);
		argv.push_back(preprocessScriptName);
		argv.push_back("--preprocessedextension");
		for (size_t i = 0; i < optionStrings_.size(); ++i) {
			argv.push_back("-r");
			argv.push_back(optionStrings_[i]);
		}
		argv.push_back("-o");
		argv.push_back(tempFile1);

	#if defined _MSC_VER
		int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
	#elif defined __GNUC__
		int r = systemv(argv.value());
	#endif
		if (r == -1) {
			return boost::optional<std::string>("error: fail to invoke preprocess.py (5)");
		}
		if (r != 0) {
			return boost::optional<std::string>(""); // error message has been already printed out by preprocess.py.py
		}
		::my_sleep(::theWaitAfterProcessInvocation);

		std::vector<std::string> lines;
		get_raw_lines(tempFile1, &lines);
		::remove(tempFile1.c_str());
		assert(lines.size() > 0);
		postfix = lines[0];
		optionStrings = optionStrings_;

		return boost::optional<std::string>();
	}
	void setTemporaryDir(const std::string &fileOnTheDir)
	{
		tempFileDir = fileOnTheDir;
	}
	void setMaxWorkerThreads(int optionMaxWorkerThreads_)
	{
		optionMaxWorkerThreads = optionMaxWorkerThreads_;
	}
	boost::optional<std::set<std::string> > getExtensionsAssociatedToPreprocessor(const std::string &preprocessor) const
	{
		boost::optional<std::set<std::string> > extensions;
		std::map<std::string/* scriptName */, std::set<std::string> /* extensions */>::const_iterator i = preprocessorExtensionTable.find(preprocessor);
		if (i != preprocessorExtensionTable.end()) {
			extensions = i->second;
		}
		return extensions;
	}
	void setPreprocessFileDirectories(const std::vector<std::string> &prepDirs_)
	{
		prepDirs = prepDirs_;
	}
	void addExtraPrepDescriptionFile(const std::string &filePath)
	{
		extraPrepScriptFiles.push_back(filePath);
		scriptTableReadingDone = false;
	}
public:
	int read_script_table(const std:: string &argv0_, std::string *pErrorMessage)
	{
		this->argv0 = argv0_;

		std::string dummy;
		if (pErrorMessage == NULL) {
			pErrorMessage = &dummy;
		}

		std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> > data;
		int r = extraPrepScriptFiles.empty() ? ::read_script_table(&data, argv0)
				: ::read_script_table(&data, argv0, extraPrepScriptFiles);
		if (r == 0) {
			const std::string &fileSep = file_separator();
			for (std::vector<std::pair<std::string/* ext */, std::string/* scriptFile */> >::const_iterator si = data.begin(); si != data.end(); ++si) {
				const std::string &extension = si->first;
				const std::string &scriptName = si->second;
				std:: string path = make_filename_on_the_same_directory(std::string("scripts") + fileSep + "pp" + fileSep + scriptName + ".py", argv0);
				PathTime pt;
				if (! PathTime::getFileMTime(path, &pt)) {
					*pErrorMessage = (boost::format("error: can't open a script file '%s'") % path).str();
					return 2;
				}

				std:: string scriptFName;
				if (! splitpath(path, NULL, &scriptFName, NULL)) {
					*pErrorMessage = "error: invalid line in the script setting file";
					return 2;
				}

				std:: map<std:: string/* preprocessor */, PathTime>::const_iterator i = preprocessorTable.find(scriptName);
				if (i != preprocessorTable.end()) {
					if (i->second.path != pt.path) {
						*pErrorMessage = "error: preprocessor name conflicts in the script setting file";
						return 2;
					}
				}
				else {
					preprocessorTable[scriptFName] = pt;
				}

				preprocessorExtensionTable[scriptName].insert(extension);
			}
			scriptTableReadingDone = true;
		}

		return r;
	}
private:
	// not used
	int getPreprocessedFileName(const std:: string &original, std:: string *pPrepFile, PathTime *pPtInput) 
	{
		assert(! preprocessScriptName.empty());
		assert(! preprocessScript.path.empty());

		PathTime &ptInput = *pPtInput;
		if (! PathTime::getFileMTime(original, &ptInput)) {
			std:: cerr << "error: can't open an input file '" << original << "'" << std:: endl;
			return 1;
		}

		std:: string prepFile = original + "." + preprocessScriptName + prepExtension;
		(*pPrepFile) = prepFile;

		return 0;
	}
public:
	int performPreprocess(const std:: vector<std:: string> &files)
	{
		return performPreprocess_i(files, NULL);
	}
	int performPreprocess(const std:: vector<std:: string> &files, std::vector<std:: string> *pErrorIncludingFiles) 
	{
		assert(pErrorIncludingFiles != NULL);
		return performPreprocess_i(files, pErrorIncludingFiles);
	}
private:
	int performPreprocess_i(const std:: vector<std:: string> &files, std::vector<std:: string> *pErrorIncludingFiles)
	{
		assert(! preprocessScriptName.empty());
		assert(! preprocessScript.path.empty());
		if (files.empty()) return 0;

		//std::cerr << "debug 2: tempFileDir = " << tempFileDir << std::endl;
		std::string tempFile1 = ::make_temp_file_on_the_same_directory(tempFileDir, "ccfx-prep", ".tmp");
		std::string tempFileErrorFiles = ::make_temp_file_on_the_same_directory(tempFileDir, "ccfx-prep", ".tmp");

		{
			FileStructWrapper pf(tempFile1, "wb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
			if (! pf) {
				std::cerr << "error: fail to create a temporary file" << std::endl;
				return 2;
			}
			for (size_t i = 0; i < files.size(); ++i) {
				fwrite(files[i].data(), sizeof(char), files[i].size(), pf);
				fputc('\n', pf);
			}
		}
		
		std::string scriptPath = make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);
		ArgvBuilder argv;
		argv.push_back(thePythonInterpreterPath);
		argv.push_back(scriptPath);
		argv.push_back(preprocessScriptName);
		if (! optionEncoding.empty()) {
			argv.push_back("-c");
			argv.push_back(optionEncoding);
		}
		for (size_t i = 0; i < optionStrings.size(); ++i) {
			argv.push_back("-r");
			argv.push_back(optionStrings[i]);
		}
		for (size_t i = 0; i < prepDirs.size(); ++i) {
			const std::string &p = prepDirs[i];
			argv.push_back("-n");
			argv.push_back(p);
		}
		argv.push_back("-i");
		argv.push_back(tempFile1);
		if (pErrorIncludingFiles != NULL) {
			argv.push_back((boost::format("--errorfiles=%s") % tempFileErrorFiles).str());
		}
		if (optionMaxWorkerThreads >= 0) {
			argv.push_back((boost::format("--threads=%d") % optionMaxWorkerThreads).str());
		}
		if (optionVerbose) {
			argv.push_back("-v");
		}
	#if defined _MSC_VER
		int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
	#elif defined __GNUC__
		int r = systemv(argv.value());
	#endif
		if (r == -1) {
			std::cerr << "error: fail to invoke preprocess.py (3)";
			return 2;
		}
		::my_sleep(::theWaitAfterProcessInvocation);

		if (r != 0) {
			return r;
		}

		::remove(tempFile1.c_str());

		if (pErrorIncludingFiles != NULL) {
			std::vector<std::string> lines;
			if (! get_raw_lines(tempFileErrorFiles, &lines)) {
				std::cerr << "error: fail to read a temporary file" << std::endl;
				return 2;
			}
			std::swap(*pErrorIncludingFiles, lines);
		}
		::remove(tempFileErrorFiles.c_str());

		return 0;
	}
public:
	int removeObsoletePreprocessedFiles(const std:: vector<std:: string> &files)
	{
		assert(! preprocessScriptName.empty());
		assert(! preprocessScript.path.empty());
		if (files.empty()) return 0;

		//std::cerr << "debug 2: tempFileDir = " << tempFileDir << std::endl;
		std::string tempFile1 = ::make_temp_file_on_the_same_directory(tempFileDir, "ccfx-prep", ".tmp");

		{
			FileStructWrapper pf(tempFile1, "wb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
			if (! pf) {
				std::cerr << "error: fail to create a temporary file" << std::endl;
				return 2;
			}
			for (size_t i = 0; i < files.size(); ++i) {
				fwrite(files[i].data(), sizeof(char), files[i].size(), pf);
				fputc('\n', pf);
			}
		}
		
		std::string scriptPath = make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);
		ArgvBuilder argv;
		argv.push_back(thePythonInterpreterPath);
		argv.push_back(scriptPath);
		argv.push_back(preprocessScriptName);
		if (! optionEncoding.empty()) {
			argv.push_back("-c");
			argv.push_back(optionEncoding);
		}
		for (size_t i = 0; i < optionStrings.size(); ++i) {
			argv.push_back("-r");
			argv.push_back(optionStrings[i]);
		}
		for (size_t i = 0; i < prepDirs.size(); ++i) {
			const std::string &p = prepDirs[i];
			argv.push_back("-n");
			argv.push_back(p);
		}
		argv.push_back("-i");
		argv.push_back(tempFile1);
		argv.push_back("--removeobsolete");
	#if defined _MSC_VER
		int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
	#elif defined __GNUC__
		int r = systemv(argv.value());
	#endif
		if (r == -1) {
			std::cerr << "error: fail to invoke preprocess.py (2)";
			return 2;
		}
		::my_sleep(::theWaitAfterProcessInvocation);

		if (r != 0) {
			return r;
		}
		::remove(tempFile1.c_str());

		return 0;
	}
public:
	std::string getPreprocessScriptName() const
	{
		assert(! preprocessScriptName.empty());
		return preprocessScriptName;
	}
	std::string getPostfix() const
	{
		assert(! postfix.empty());

		return postfix;
	}
	int getPreprocessedFileName(const std:: string &original, std:: string *pPrepFile)
	{
		assert(! postfix.empty());
		*pPrepFile = original + postfix;
		
		return 0;
	}
	boost::optional<std::string/* error message */ > getDefaultParameterizing(std::map<std::string, int> *pParameterizing)
	{
		assert(pParameterizing != NULL);
		assert(scriptTableReadingDone);
		assert(! preprocessScriptName.empty());

		std::string tempFile1 = ::make_temp_file_on_the_same_directory(tempFileDir, "ccfx-prep", ".tmp");

		std::string scriptPath = ::make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);
		ArgvBuilder argv;
		argv.push_back(thePythonInterpreterPath);
		argv.push_back(scriptPath);
		argv.push_back(preprocessScriptName);
		argv.push_back("--getdefaultparameterizing");
		argv.push_back("-o");
		argv.push_back(tempFile1);
	#if defined _MSC_VER
		int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
	#elif defined __GNUC__
		int r = systemv(argv.value());
	#endif
		if (r == -1) {
			return boost::optional<std::string>("error: fail to invoke preprocess.py (4)");
		}
		if (r != 0) {
			return boost::optional<std::string>(""); // error message has been already printed out by preprocess.py
		}
		::my_sleep(::theWaitAfterProcessInvocation);

		std::vector<std::string> lines;
		get_raw_lines(tempFile1, &lines);
		::remove(tempFile1.c_str());

		for (size_t i = 0; i < lines.size(); ++i) {
			const std::string &line = lines[i];
			std::vector<std::string> fields;
			boost::algorithm::split(fields, line, boost::algorithm::is_any_of("\t"));
			if (fields.size() != 2) {
				return boost::optional<std::string>("error: invalid parameterizing data");
			}
			const std::string &tokenName = fields[0];
			const std::string &value = fields[1];
			int pramOperation = 0;
			if (value == "ANY_MATCH") {
				pramOperation = Param::ANY_MATCH;
			}
			else if (value == "EXACT_MATCH") {
				pramOperation = Param::EXACT_MATCH;
			}
			else if (value == "P_MATCH") {
				pramOperation = Param::P_MATCH;
			}
			else {
				return boost::optional<std::string>("error: invalid parameterizing data");
			}
			(*pParameterizing)[tokenName] = pramOperation;
		}

		return boost::optional<std::string>();
	}
};

#endif // defined PREPROCESSORINVOKER_H
