#include <string>

#include <boost/format.hpp>

#include "prettyprintmain.h"
#include "clonedataassembler.h"

bool PrettyPrintMain::assemble(const std::string &outputFile, const std::string &inputFile, std::string *pErrorMessage)
{
	CloneDataAssembler cda;
	try {
		cda.assemble(outputFile, inputFile);
		return true;
	}
	catch (CloneDataAssembler::Error &e) {
		if (pErrorMessage != NULL) {
			if (e.lineNumber == 0) {
				*pErrorMessage = e.what();
			}
			else {
				*pErrorMessage = (boost::format("line %d: %s") % e.lineNumber % e.what()).str();
			}
		}
		return false;
	}
}
