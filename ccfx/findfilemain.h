
#include <cassert>
#include <cstdlib>
#include <map>
#include <set>
#include "../common/hash_map_includer.h"
#include "../common/hash_set_includer.h"
#include <vector>

#include "../common/unportable.h"
#include "../common/argvbuilder.h"

#if defined _MSC_VER
#include "../common/win32util.h"
#endif

#include "preprocessorinvoker.h"
#include "ccfxcommon.h"

#if defined __GNUC__

#include <unistd.h>

#elif defined _MSC_VER

#include <direct.h>
#define getcwd _getcwd
#include <process.h>

#else

#error "unknown compiler"

#endif

class FindFileMain {
private:
	std::string argv0;
	std:: map<std:: string/* preprocessor */, std:: set<std:: string> > preprocessorTable;
	bool optionConvertToAbsolutePath;
	bool optionPrintPreprocessedFilesDirectories;
	bool optionPrintGroupSeparator;

private:
//	int making_pack_files(const std::vector<std::string> &directoryPaths) 
//	{
//		std::string argv0 = script_table_reader::getArgv0();
//		std::string scriptPath = ::make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);
//		ArgvBuilder argv;
//		argv.push_back(thePythonInterpreterPath);
//		argv.push_back(scriptPath);
//		argv.push_back("--makepack");
//		for (size_t i = 0; i < directoryPaths.size(); ++i) {
//			argv.push_back(directoryPaths[i]);
//		}
//#if defined _MSC_VER
//		int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
//#elif defined __GNUC__
//		int r = systemv(argv.value());
//#endif
//		if (r == -1) {
//			std::cerr << "error: fail to invoke preprocess.py";
//			return 2;
//		}
//		if (r != 0) {
//			return r;
//		}
//		::my_sleep(::theWaitAfterProcessInvocation);
//		return 0;
//	}
public:
	int main(const std::vector<std::string> &argv)
	{
		int r = readScriptTable(argv[0]);
		if (r != 0) {
			return r;
		}
		assert(! argv0.empty());

		assert(argv.size() >= 2);
		if (argv.size() == 2 || argv[2] == "-h" || argv[2] == "--help") {
			std:: cout << 
				"Usage 1: ccfx F PREPROCESS_SCRIPT OPTIONS directories..." "\n"
				"  Finds files that matches the preprocess_script." "\n"
				"Option" "\n"
				"  -a: prints out each file in full path (absolute path)." "\n"
				"  -e: prints out extensions that are associated with the preprocess script." "\n"
				"  -l n: generates lines for preprocessed-file directories." "\n"
				"  -l is: generates lines for group separators." "\n"  
				"  -o out.txt: output file." "\n"
				"  --listoptions: prints out preprocess options." "\n"
				"  --preprocessedextension: prints out extension of preprocessed file." "\n"
				"  --getdefaultparameterizing: prints out default matching option for each parameter names." "\n"
				"Usage 2: ccfx F -c [-o output]" "\n"
				"  Prints out available encodings." "\n"
				"Usage 3: ccfx F -p [-o output]" "\n"
				"  Prints out names of the available preprocess scripts." "\n"
				"Usage 4: ccfx F -n [-a] [-o output] directories..." "\n"
				"  Prints out preprocessed-file directories." "\n"
				//"Usage 3: ccfx F --makepack directories..." "\n"
				//"  Make pack file (.ccfxprep.pack) in the directories." "\n"
				;
			return 0;
		}

		std:: string outputFile;
		optionConvertToAbsolutePath = false;
		optionPrintPreprocessedFilesDirectories = false;
		optionPrintGroupSeparator = false;
		
		enum { MODE_NONE = 0,
			MODE_FINDFILES,
			MODE_PRINTPROCESSORS, 
			MODE_PRINTEXTENSIONS, 
			MODE_PRINTAVAILABLEENCODINGS, 
			MODE_PRINTPREPROCESSOPTIONDESCRIPTION, 
			MODE_PRINTPREPROCESSEDFILEEXTENSION,
			MODE_PRINTDEFAULTPARAMETERIZING,
			MODE_FINDREPROCESSFILEDIRECTORIES 
		};

		int mode = MODE_NONE;

		//if (2 < argv.size()) {
		//	std::string argi = argv[2];
		//	if (argi == "--makepack") {
		//		std::vector<std::string> args;
		//		for (size_t i = 3; i < argv.size(); ++i) {
		//			args.push_back(argv[i]);
		//		}
		//		return making_pack_files(args);
		//	}
		//}

		std::vector<std::string> args;
		std::vector<std::string> preprocessOptions;

		for (size_t i = 2; i < argv.size(); ++i) {
			std:: string argi = argv[i];

			if (! argi.empty() && argi[0] == '-') {
				if (argi == "-o") {
					if (! (i + 1 < argv.size())) {
						std:: cerr << "error: option -o requires an argument" << std:: endl;
						return 1;
					}
					outputFile = argv[i + 1];
					++i;
				}
				else if (argi == "-a") {
					optionConvertToAbsolutePath = true;
				}
				else if (argi == "-l") {
					if (! (i + 1 < argv.size())) {
						std::cerr << "error: option -l requires an argument" << std::endl;
						return 1;
					}
					std::string a = argv[i + 1];
					if (a == "n") {
						optionPrintPreprocessedFilesDirectories = true;
					}
					else if (a == "is") {
						optionPrintGroupSeparator = true;
					}
					++i;
				}
				else if (argi == "--listoptions") {
					mode = MODE_PRINTPREPROCESSOPTIONDESCRIPTION;
				}
				else if (argi == "--preprocessedextension") {
					mode = MODE_PRINTPREPROCESSEDFILEEXTENSION;
				}
				else if (argi == "--getdefaultparameterizing") {
					mode = MODE_PRINTDEFAULTPARAMETERIZING;
				}
				else if (argi == "-r") {
					if (mode != MODE_PRINTPREPROCESSEDFILEEXTENSION) {
						std::cerr << "error: option -r is valid only with option --preprocessedextension" << std::endl;
						return 1;
					}
					if (! (i + 1 < argv.size())) {
						std::cerr << "error: option -r requires an argument" << std::endl;
						return 1;
					}
					preprocessOptions.push_back(argv[i + 1]);
					++i;
				}
				else if (std::string("-c-d-e-n-p").find(argi) != std::string::npos) {
					if (mode != MODE_NONE) {
						std:: cerr << "error: option -c, -d, -e, -n, and -p are exclusive" << std:: endl;
						return 1;
					}
					if (argi == "-c") {
						mode = MODE_PRINTAVAILABLEENCODINGS;
					} 
					else if (argi == "-d") {
						mode = MODE_PRINTPREPROCESSOPTIONDESCRIPTION;
					} 
					else if (argi == "-e") {
						mode = MODE_PRINTEXTENSIONS;
					} 
					else if (argi == "-p") {
						mode = MODE_PRINTPROCESSORS;
					} 
					else if (argi == "-n") {
						mode = MODE_FINDREPROCESSFILEDIRECTORIES;
					} 
					else {
						std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
						return 1;
					}
				}
				else {
					std:: cerr << "error: unknown option '" << argi << "'" << std:: endl;
					return 1;
				}
			}
			else {
				args.push_back(argi);
			}
		}
		
		if (mode == MODE_NONE) {
			if (args.size() == 0) {
				mode = MODE_PRINTPROCESSORS;
			}
			else {
				mode = MODE_FINDFILES;
			}
		}

		std::string script;
		std:: vector<std:: string> directories;
		switch (mode) {
		case MODE_PRINTPREPROCESSOPTIONDESCRIPTION:
		case MODE_FINDFILES:
		case MODE_PRINTEXTENSIONS:
		case MODE_PRINTPREPROCESSEDFILEEXTENSION:
		case MODE_PRINTDEFAULTPARAMETERIZING:
			if (args.empty()) {
				std::cerr << "error: no preprocess script name is given" << std::endl;
				return 1;
			}
			script = args[0];
			args.erase(args.begin());
			break;
		}
		directories.insert(directories.end(), args.begin(), args.end());

		switch (mode) {
		case MODE_PRINTPREPROCESSEDFILEEXTENSION:
			{
				std::string postfix;
				std::string errorMessage;
				if (! getExtensionOfPreprocessedFile(&postfix, script, preprocessOptions, &errorMessage)) {
					if (! errorMessage.empty()) {
						std::cerr << errorMessage;
					}
					else {
						std::cerr << "error: in invocation of preprocess.py" << std::endl;
					}
					return 1;
				}

				std:: ostream *pOutput = &std:: cout;
				
				std:: ofstream output;
				if (! outputFile.empty()) {
					output.open(outputFile.c_str());
					if (! output.is_open()) {
						std:: cerr << "error: can't open file '" << outputFile << "'" << std:: endl;
						return 1;
					}
					pOutput = &output;
				}

				(*pOutput) << postfix << std::endl;

				if (! outputFile.empty()) {
					output.close();
				}

				return 0;
			}
			break;
		case MODE_PRINTDEFAULTPARAMETERIZING:
			{
				std::map<std::string, int> defaultParameterizing;
				std::string errorMessage;
				if (! this->getDefaultParameterizing(&defaultParameterizing, script, &errorMessage)) {
					if (! errorMessage.empty()) {
						std::cerr << errorMessage;
					}
					else {
						std::cerr << "error: in invocation of preprocess.py" << std::endl;
					}
					return 1;
				}

				std:: ostream *pOutput = &std:: cout;
				
				std:: ofstream output;
				if (! outputFile.empty()) {
					output.open(outputFile.c_str());
					if (! output.is_open()) {
						std:: cerr << "error: can't open file '" << outputFile << "'" << std:: endl;
						return 1;
					}
					pOutput = &output;
				}
				
				for (std::map<std::string, int>::const_iterator i = defaultParameterizing.begin(); i != defaultParameterizing.end(); ++i) {
					const std::string &tokenName = i->first;
					int paramOperation = i->second;
					(*pOutput) << tokenName << "\t" << Param::toName(paramOperation) << std::endl;
				}

				if (! outputFile.empty()) {
					output.close();
				}

				return 0;
			}
			break;
		case MODE_PRINTPREPROCESSOPTIONDESCRIPTION:
			{
				assert(! script.empty());
				assert(! argv0.empty());
				std::string scriptPath = ::make_filename_on_the_same_directory("scripts" + file_separator() + "preprocess.py", argv0);
				ArgvBuilder argv;
				argv.push_back(thePythonInterpreterPath);
				argv.push_back(scriptPath);
				argv.push_back(script);
				argv.push_back("--listoptions");
	#if defined _MSC_VER
				int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
	#elif defined __GNUC__
				int r = systemv(argv.value());
	#endif
				if (r == -1) {
					std::cerr << "error: fail to invoke preprocess.py (1)";
					return 2;
				}
				if (r != 0) {
					return r;
				}

				return 0;
			}
			break;
		case MODE_PRINTEXTENSIONS:
			{
				assert(! script.empty());
				std:: ostream *pOutput = &std:: cout;
				
				std:: ofstream output;
				if (! outputFile.empty()) {
					output.open(outputFile.c_str());
					if (! output.is_open()) {
						std:: cerr << "error: can't open file '" << outputFile << "'" << std:: endl;
						return 1;
					}
					pOutput = &output;
				}
				
				std:: map<std:: string/* preprocessor */, std:: set<std:: string> >::iterator i = preprocessorTable.find(script);
				if (i == preprocessorTable.end()) {
					std:: cerr << "error: unknown preprocess script '" << script << "'" << std:: endl;
					return 1;
				}

				const std:: set<std:: string> &extensions = i->second;
				for (std:: set<std:: string>::const_iterator j = extensions.begin(); j != extensions.end(); ++j) {
					(*pOutput) << (*j) << std:: endl;
				}

				if (! outputFile.empty()) {
					output.close();
				}

				return 0;
			}
			break;
		case MODE_PRINTAVAILABLEENCODINGS:
			{
				std::string scriptPath = ::make_filename_on_the_same_directory("scripts" + file_separator() + "easytorq_helper.py", argv0);
				ArgvBuilder argv;
				argv.push_back(thePythonInterpreterPath);
				argv.push_back(scriptPath);
				argv.push_back("-c");
#if defined _MSC_VER
				int r = _spawnv(_P_WAIT, thePythonInterpreterPath.c_str(), argv.c_argv());
#elif defined __GNUC__
				int r = systemv(argv.value());
#endif
				if (r == -1) {
					std::cerr << "error: fail to invoke easytorq_helper.py";
					return 2;
				}
				if (r != 0) {
					return r;
				}
				::my_sleep(::theWaitAfterProcessInvocation);

				return 0;
			}
			break;
		case MODE_PRINTPROCESSORS:
			{
				std:: ostream *pOutput = &std:: cout;
				
				std:: ofstream output;
				if (! outputFile.empty()) {
					output.open(outputFile.c_str());
					if (! output.is_open()) {
						std:: cerr << "error: can't open file '" << outputFile << "'" << std:: endl;
						return 1;
					}
					pOutput = &output;
				}
				
				for (std:: map<std:: string/* preprocessor */, std:: set<std:: string> >::iterator i = preprocessorTable.begin(); i != preprocessorTable.end(); ++i) {
					(*pOutput) << i->first << std:: endl;
				}

				return 0;
			}
			break;
		case MODE_FINDREPROCESSFILEDIRECTORIES:
			// do nothing
			break;
		case MODE_FINDFILES:
			// do nothing
			break;
		default:
			assert(false);
			break;
		}

		assert(mode == MODE_FINDFILES || mode == MODE_FINDREPROCESSFILEDIRECTORIES);
		
		if (mode == MODE_FINDFILES) {
			if (preprocessorTable.find(script) == preprocessorTable.end()) {
				std:: cerr << "error: unknown script identifier '" << script << "'" << std:: endl;
				return 1;
			}
		}
		else if (mode == MODE_FINDREPROCESSFILEDIRECTORIES) {
			// do nothing
		}
		else {
			assert(false);
		}


		if (directories.empty()) {
			directories.push_back(".");
		}
		if (optionConvertToAbsolutePath) {
			char *p = getcwd(NULL, 0);
			if (p == NULL) {
				std:: cerr << "error: fail to get current directory" << std:: endl;
				return 1;
			}
			std:: string curDir(p);
			free(p);
//#if defined OS_WIN32
//			if (curDir.length() > 0 && ! (curDir[curDir.length() - 1] == '/' || curDir[curDir.length() - 1] == '\\')) {
//#elif defined OS_UNIX
//			if (curDir.length() > 0 && curDir[curDir.length() - 1] != '/') {
//#endif
//				curDir += ::file_separator();
//			}
			for (size_t i = 0; i < directories.size(); ++i) {
				std:: string &di = directories[i];
				if (di == ".") {
					di = curDir;
				}
//				else
//#if defined OS_WIN32
//				if (boost::starts_with(di, ".\\") || boost::starts_with(di, "./")) {
//#elif defined OS_UNIX
//				if (boost::starts_with(di, "./")) {
//#endif
//					di = curDir + di.substr(2);
//				}
			}
		}

		std:: vector<std:: string> files;
		if (mode == MODE_FINDFILES) {
			assert(! script.empty());
			if (optionPrintPreprocessedFilesDirectories) {
				for (size_t i = 0; i < directories.size(); ++i) {
					files.push_back((boost::format("-n %s") % directories[i]).str());
				}
			}
			for (size_t i = 0; i < directories.size(); ++i) {
				if (i >= 1 && optionPrintGroupSeparator) {
					files.push_back("-is");
				}
				std:: vector<std:: string> fs;
				if (! find_files(&fs, preprocessorTable[script], directories[i])) {
					std:: cerr << "error: can't change directory to '" << directories[i] << "'" << std:: endl;
					return 1;
				}
				files.insert(files.end(), fs.begin(), fs.end());
			}
		}
		else if (mode == MODE_FINDREPROCESSFILEDIRECTORIES) {
			std::set<std::string> names;
			names.insert(".ccfxprepdir");
			for (size_t i = 0; i < directories.size(); ++i) {
				std:: vector<std:: string> fs;
				if (! find_named_directories(&fs, names, directories[i])) {
					std:: cerr << "error: can't change directory to '" << directories[i] << "'" << std:: endl;
					return 1;
				}
				files.insert(files.end(), fs.begin(), fs.end());
			}
		}
		else {
			assert(false);
		}

		if (! outputFile.empty()) {
			std:: ofstream output;
			output.open(outputFile.c_str());
			if (! output.is_open()) {
				std:: cerr << "error: can't open file '" << outputFile << "'" << std:: endl;
				return 1;
			}
			for (size_t i = 0; i < files.size(); ++i) {
				const std:: string &file = files[i];
				bool isFileNameSuccessfullyConvertedToUtf8 = true;
				{
					const std::string encFile = SYS2INNER(file);
					const std::string decFile = INNER2SYS(encFile);
					if (decFile != file) {
						isFileNameSuccessfullyConvertedToUtf8 = false;
					}
				}
				if (isFileNameSuccessfullyConvertedToUtf8) {
					output.write(file.data(), file.length());
					output << std:: endl;
				}
				else {
					std::cerr << "warning: invalid file name: " << file << std::endl;
				}
			}
		}
		else {
			for (size_t i = 0; i < files.size(); ++i) {
				const std:: string &file = files[i];
				std:: cout.write(file.data(), file.length());
				std:: cout << std:: endl;
			}
		}

		return 0;
	}
protected:
	int readScriptTable(const std:: string &argv0_)
	{
		this->argv0 = argv0_;

		preprocessorTable.clear();
		
		std::vector<std::pair<std::string, std::string> > data;
		int r = read_script_table(&data, argv0);
		if (r != 0) {
			return r;
		}
		for (std::vector<std::pair<std::string, std::string> >::const_iterator si = data.begin(); si != data.end(); ++si) {
			const std::string &extension = si->first;
			const std::string &scriptName = si->second;
			preprocessorTable[scriptName].insert(extension);
		}

		return 0;
	}
private:
	static bool getExtensionOfPreprocessedFile(std::string *pPostfix, const std::string &preprocessScript, const std::vector<std::string> &preprocessOptions, 
			std::string *pErrorMessage)
	{
		assert(pPostfix != NULL);
		assert(pErrorMessage != NULL);

		preprocessor_invoker prepInvoker;
		int r = prepInvoker.read_script_table(theArgv0, pErrorMessage);
		if (r != 0) {
			return false;
		}
		prepInvoker.setPreprocessorName(preprocessScript);
		boost::optional<std::string> errorMessage = prepInvoker.setPreprocessorOptions(preprocessOptions);
		if (errorMessage) {
			*pErrorMessage = *errorMessage;
			return false;
		}
		*pPostfix = prepInvoker.getPostfix();
		return true;
	}
	static bool getDefaultParameterizing(std::map<std::string, int> *pParameterizing, const std::string &preprocessScript,
			std::string *pErrorMessage)
	{
		assert(pParameterizing != NULL);
		assert(pErrorMessage != NULL);

		preprocessor_invoker prepInvoker;
		int r = prepInvoker.read_script_table(theArgv0, pErrorMessage);
		if (r != 0) {
			return false;
		}
		prepInvoker.setPreprocessorName(preprocessScript);
		boost::optional<std::string> errorMessage = prepInvoker.getDefaultParameterizing(pParameterizing);
		if (errorMessage) {
			*pErrorMessage = *errorMessage;
			return false;
		}
		return true;
	}
};

