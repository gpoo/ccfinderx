#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include <iostream>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>

#include "../common/datastructureonfile.h"

int convert_binary_file_to_text(const std:: string &file)
{
	FILE *pf = fopen(file.c_str(), "rb");
	if (pf == NULL) {
		std:: cerr << "error: can not open file '" << file << "'" << std:: endl;
		return 1;
	}

	while (true) {
		long long buf;
		if (fread(&buf, sizeof(long long), 1, pf) != 1) {
			break; // while true
		}
		std:: cout << buf << std:: endl;
	}

	fclose(pf);

	return 0;
}

bool get_raw_lines(const std:: string &file_path, std:: vector<std:: string> *pLines)
{
	std:: ifstream is;
	is.open(file_path.c_str(), std:: ios::in | std:: ios::binary);
	if (! is.is_open()) {
		return false;
	}
	
	std:: vector<std:: string> lines;
	std:: string str;
	while (! is.eof()) {
		std:: getline(is, str, '\n');
		if (str.length() >= 1 && str[str.length() - 1] == '\r') {
			str.resize(str.length() - 1);
		}
		lines.push_back(str);
	}
	is.close();
	if (lines.size() >= 1 && lines[lines.size() - 1].length() == 0) {
		lines.resize(lines.size() - 1);
	}

	(*pLines).swap(lines);

	return true;	
}

enum Operator {
	OP_NULL,
	OP_LT, // < or .lt.
	OP_LE, // <= or .le.
	OP_NE, // != or <> or .ne.
	OP_EQ, // = or == or .eq.
	OP_GE, // >= or .ge.
	OP_GT, // > or .gt.
};

struct Condition {
public:
	std:: string col;
	Operator ope;
	long double num;
};

class OperatorTable {
private:
	std:: map<std:: string, Operator> str2ope;
public:
	OperatorTable()
	{
		static struct { const char *str; Operator ope; } tbl[] = {
			{ "<",  OP_LT }, { ".lt.", OP_LT }, { ".LT.", OP_LT },
			{ "<=", OP_LE }, { ".le.", OP_LE }, { ".LE.", OP_LE },
			{ "!=", OP_NE }, { "<>",   OP_NE }, { ".ne.", OP_NE }, { ".NE.", OP_NE },
			{ "=",  OP_EQ }, { "==",   OP_EQ }, { ".eq.", OP_EQ }, { ".EQ.", OP_EQ },
			{ ">=", OP_GE }, { ".ge.", OP_GE }, { ".GE.", OP_GE },
			{ ">",  OP_GT }, { ".gt.", OP_GT }, { ".GT.", OP_GT },
			{ NULL, OP_NULL }
		};
		for (size_t i = 0; tbl[i].str != NULL; ++i) {
			str2ope[tbl[i].str] = tbl[i].ope;
		}
	}
	Operator toOperator(const std:: string &str) const
	{
		std:: map<std:: string, Operator>::const_iterator i = str2ope.find(str);
		if (i == str2ope.end()) {
			return OP_NULL;
		}
		return i->second;
	}
};

void split(std:: vector<std:: string> *pResult, const std:: string &str, const std:: string &sep)
{
	(*pResult).clear();

	size_t i = 0;
	while (i < str.length()) {
		size_t p = str.find(sep, i);
		if (p == std:: string::npos) {
			(*pResult).push_back(str.substr(i));
			return;
		}
		(*pResult).push_back(str.substr(i, p - i));
		assert(p + 1 > i);
		i = p + 1;
	}

	(*pResult).push_back("");
	return;
}

class Main
{
private:
	struct INDSELORD {
	public:
		long double sel;
		long double ord;
	public:
		INDSELORD(long double sel_, long double ord_)
			: sel(sel_), ord(ord_)
		{
		}
		INDSELORD()
			: sel(0), ord(0)
		{
		}
	public:
		struct ComparatorAsc {
		public:
			inline bool operator()(const INDSELORD &left, const INDSELORD &right) const
			{
				if (left.ord < right.ord) {
					return true;
				}
				return false;
			}
		};
		struct ComparatorDesc {
		public:
			inline bool operator()(const INDSELORD &left, const INDSELORD &right) const
			{
				if (left.ord > right.ord) {
					return true;
				}
				return false;
			}
		};
	};
		
private:
	std:: string modulePath;
	std:: string outputFile;
	std:: string fromFile;
	std:: string selectCol;
	
	std:: vector<Condition> conditions;
	std:: string orderByAsc; // è∏èá
	std:: string orderByDesc; // ç~èá

	long long binaryOutputFactor;

	std:: vector<std:: string> columnNames;

	std:: vector<std:: vector<std:: pair<Condition, int /* denom index */> > > conditionsForColumn;
public:
	Main()
		: binaryOutputFactor(0)
	{
	}

	int main(int argc, char *argv[])
	{
		modulePath = argv[0];

		std::vector<std::string> argvec;
		for (size_t i = 0; i < argc; ++i) {
			argvec.push_back(std::string(argv[i]));
		}

		return main_i(argvec);
	}
	int main(int argc, const char *argv[])
	{
		modulePath = argv[0];

		std::vector<std::string> argvec;
		for (size_t i = 0; i < argc; ++i) {
			argvec.push_back(std::string(argv[i]));
		}

		return main_i(argvec);
	}
private:
	int check_expression()
	{
		if (! orderByAsc.empty()) {
			int colIndex = findNamedColumn(orderByAsc);
			if (colIndex == -1) {
				std:: cerr << "error: unknown column name '" << orderByAsc << "'" << std:: endl;
				return 1;
			}
		}
		else if (! orderByDesc.empty()) {
			int colIndex = findNamedColumn(orderByDesc);
			if (colIndex == -1) {
				std:: cerr << "error: unknown column name '" << orderByDesc << "'" << std:: endl;
				return 1;
			}
		}
		else {
			conditionsForColumn.resize(columnNames.size());
			for (size_t i = 0; i < conditions.size(); ++i) {
				const Condition &cond = conditions[i];
				int colIndex = findNamedColumn(cond.col);
				if (colIndex == -1) {
					std:: cerr << "error: unknown column name '" << cond.col << "'" << std:: endl;
					return 1;
				}
				conditionsForColumn[colIndex].push_back(std:: pair<Condition, int>(cond, -1));
			}
		}

		return 0;
	}
	int do_sorting(std:: istream *pInput, std:: ostream *pOutput, int selectColIndex, int orderColIndex, int order,
			const std:: string &outputFile)
	{
		std:: string tempUnsorted = make_temp_file_on_the_same_directory(outputFile, "picoseltmp0", ".tmp");
		std:: string temp1 = make_temp_file_on_the_same_directory(outputFile, "picoseltmp1", ".tmp");
		std:: string temp2 = make_temp_file_on_the_same_directory(outputFile, "picoseltmp2", ".tmp");
		std:: string tempSorted = make_temp_file_on_the_same_directory(outputFile, "picoseltmp3", ".tmp");
		
		unsigned long long int itemCount;
		{
			FILE *pfTemp0 = fopen(tempUnsorted.c_str(), "wb" F_TEMPORARY_FILE_OPTIMIZATION);
			if (pfTemp0 == NULL) {
				std:: cerr << "error: can not create a temporary file" << std:: endl;
				return 1;
			}

			std:: vector<long double> colValues;
			colValues.resize(columnNames.size());

			itemCount = 0;
			unsigned long long lineNumber = 1;
			while (! (*pInput).eof()) {
				std:: string str;
				std:: getline((*pInput), str, '\n');
				if (str.length() == 0 && (*pInput).eof()) {
					break; // while
				}
				if (str.length() >= 1 && str[str.length() - 1] == '\r') {
					str.resize(str.length() - 1);
				}
				++lineNumber;
				std:: vector<std:: string> cols;
				split(&cols, str, "\t");
				if (cols.size() != columnNames.size()) {
					std:: cerr << "error: line #" << lineNumber << ": number of colums differs" << std:: endl;
					return 1;
				}
				
				bool headdingLine = false;
				for (size_t i = 0; i < cols.size(); ++i) {
					if (! str_to_ld(&colValues[i], cols[i])) {
						headdingLine = true;
						break; // for i
					}
				}

				if (! headdingLine) {
					INDSELORD selord(colValues[selectColIndex], colValues[orderColIndex]);
					fwrite(&selord, sizeof(INDSELORD), 1, pfTemp0);
					++itemCount;
				}
			}

			fclose(pfTemp0);
		}

		{
			onfile::Sorter<INDSELORD> sorter;
			sorter.setTempFileNames(temp1, temp2);
			sorter.setBlockSize(100000);
			if (order > 0) {
				INDSELORD::ComparatorAsc comparator;
				sorter.stableSort(tempSorted, tempUnsorted, (boost::int64_t)0, itemCount, comparator);
			}
			else {
				INDSELORD::ComparatorDesc comparator;
				sorter.stableSort(tempSorted, tempUnsorted, (boost::int64_t)0, itemCount, comparator);
			}
			sorter.removeTempFiles();
		}
		
		::remove(tempUnsorted.c_str());

		{
			FILE *pfTemp0 = fopen(tempSorted.c_str(), "rb" F_SEQUENTIAL_ACCESS_OPTIMIZATION);
			if (pfTemp0 == NULL) {
				std:: cerr << "error: can not open a temporary file" << std:: endl;
				return 1;
			}

			INDSELORD selord;
			for (unsigned long long i = 0; i < itemCount; ++i) {
				fread(&selord, sizeof(INDSELORD), 1, pfTemp0);
				if (binaryOutputFactor != 0) {
					long long value = (long long)(selord.sel * binaryOutputFactor);
					(*pOutput).write((const char *)(void *)&value, sizeof(long long));
				}
				long double d = selord.sel - (long double)((long long)selord.sel);
				if (d == 0) {
					(*pOutput) << (long long)selord.sel << std:: endl;
				}
				else {
					(*pOutput) << selord.sel << std:: endl;
				}
			}

			fclose(pfTemp0);
		}

		::remove(tempSorted.c_str());

		return 0;
	}
	int do_selecting(std:: istream *pInput, std:: ostream *pOutput, int selectColIndex)
	{
		std:: vector<long double> colValues;
		colValues.resize(columnNames.size());

		unsigned long long lineNumber = 1;
		while (! (*pInput).eof()) {
			std:: string str;
			std:: getline((*pInput), str, '\n');
			if (str.length() == 0 && (*pInput).eof()) {
				break; // while
			}
			if (str.length() >= 1 && str[str.length() - 1] == '\r') {
				str.resize(str.length() - 1);
			}
			++lineNumber;
			std:: vector<std:: string> cols;
			split(&cols, str, "\t");
			if (cols.size() != columnNames.size()) {
				std:: cerr << "error: line #" << lineNumber << ": number of colums differs" << std:: endl;
				return 1;
			}
			
			bool headdingLine = false;
			for (size_t i = 0; i < cols.size(); ++i) {
				if (! str_to_ld(&colValues[i], cols[i])) {
					headdingLine = true;
					break; // for i
				}
			}

			if (! headdingLine) {
				bool satisfiesConditions = true;
				for (size_t i = 0; i < colValues.size(); ++i) {
					long double value = colValues[i];
					const std:: vector<std:: pair<Condition, int> > &conds = conditionsForColumn[i];
					for (size_t j = 0; j < conds.size(); ++j) {
						const Condition &cond = conds[j].first;
						if (! satisfies(value, cond)) {
							satisfiesConditions = false;
							break; // for j
						}
					}
					if (! satisfiesConditions) {
						break; // for i
					}
				}

				if (satisfiesConditions) {
					if (binaryOutputFactor != 0) {
						long long value = (long long)(colValues[selectColIndex] * binaryOutputFactor);
						(*pOutput).write((const char *)(void *)&value, sizeof(long long));
					}
					else {
						(*pOutput) << cols[selectColIndex] << std:: endl;
					}
				}
			}
		}

		return 0;
	}
	int main_i(const std::vector<std::string> &argv)
	{
		int argc = argv.size();
		if (argc == 1 || argc == 2 && argv[1] == "-h") {
			std:: cout << "Picosel ver. 1.5 (C) 2009-2010 AIST" "\n"
				"Usage 1: picosel OPTION from inputfile select column where EXPRESSION" "\n"
				"Usage 2: picosel OPTION from inputfile select column order by column" "\n"
				"Option" "\n"
				"  -o outputfile: specify output file." "\n";
			return 0;
		}

		if (argc == 3 && argv[1] == "--text") {
			return convert_binary_file_to_text(argv[2]);
		}

		int r = analyze_commandline(argv);
		if (r != 0) {
			return r;
		}

		std:: ifstream input;
		input.open(fromFile.c_str(), std:: ios::in | std:: ios::binary);
		if (! input.is_open()) {
			std:: cerr << "error: can not open file '" << fromFile << "'" << std:: endl;
			return 1;
		}
		std:: istream *pInput = &input;

		r = get_column_names(pInput);
		if (r != 0) {
			return r;
		}
		
		int selectColIndex = findNamedColumn(selectCol);
		if (selectColIndex == -1) {
			std:: cerr << "error: unknown column name '" << selectCol << "'" << std:: endl;
			return 1;
		}

		r = check_expression();
		if (r != 0) {
			return r;
		}

		std:: ostream *pOutput = &std:: cout;
		std:: ofstream output;
		if (! outputFile.empty()) {
			output.open(outputFile.c_str(), binaryOutputFactor != 0 ? (std:: ios::out | std:: ios::binary) : std:: ios::out);
			if (! output.is_open()) {
				std:: cerr << "error: can not create file '" << outputFile << "'" << std:: endl;
				return 1;
			}
			pOutput = &output;
		}

		if (! orderByAsc.empty()) {
			int orderColIndex = findNamedColumn(orderByAsc);
			if (orderColIndex == -1) {
				std:: cerr << "error: unknown column name '" << orderByAsc << "'" << std:: endl;
				return 1;
			}
			r = do_sorting(pInput, pOutput, selectColIndex, orderColIndex, 1/* asc */, outputFile);
		}
		else if (! orderByDesc.empty()) {
			int orderColIndex = findNamedColumn(orderByDesc);
			if (orderColIndex == -1) {
				std:: cerr << "error: unknown column name '" << orderByDesc << "'" << std:: endl;
				return 1;
			}
			r = do_sorting(pInput, pOutput, selectColIndex, orderColIndex, -1/* desc */, outputFile);
		}
		else {
			r = do_selecting(pInput, pOutput, selectColIndex);
		}

		output.close();
		input.close();

		return r;
	}
private:
	static bool satisfies(long double value, const Condition &cond)
	{
		switch (cond.ope) {
		case OP_LT:
			return value < cond.num;
		case OP_LE:
			return value <= cond.num;
		case OP_NE:
			return value != cond.num;
		case OP_EQ:
			return value == cond.num;
		case OP_GE:
			return value >= cond.num;
		case OP_GT:
			return value > cond.num;
		default:
			assert(false);
		}
		return false; // dummy
	}
	int findNamedColumn(const std:: string &name)
	{
		for (size_t j = 0; j < columnNames.size(); ++j) {
			if (columnNames[j] == name) {
				return j;
			}
		}
		return -1; // not found
	}
	int get_column_names(std:: istream *pInput)
	{
		std:: istream &input = *pInput;

		std:: string str;
		if (input.eof()) {
			std:: cerr << "error: no column names found" << std:: endl;
			return false;
		}

		std:: getline(*pInput, str, '\n');
		if (str.length() >= 1 && str[str.length() - 1] == '\r') {
			str.resize(str.length() - 1);
		}

		split(&columnNames, str, "\t");
		
		for (size_t i = 0; i < columnNames.size(); ++i) {
			std:: string s = columnNames[i];
			int j = 0;
			int ch;
			while (j < s.length() && ('a' <= (ch = s[j]) && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_')) {
				++j;
			}
			if (j == 0) {
				std:: cerr << "error: invalid name for column #" << (i + 1) << std:: endl;
				return 1;
			}
			columnNames[i] = s.substr(0, j);
		}

		for (size_t i = 0; i < columnNames.size() - 1; ++i) {
			for (size_t j = i + 1; j < columnNames.size(); ++j) {
				if (columnNames[i] == columnNames[j]) {
					std:: cerr << "error: column #" << (i + 1) << " and column #" << (j + 1) << " have the same name" << std:: endl;
					return 1;
				}
			}
		}

		return 0;
	}
	int analyze_commandline(const std::vector<std::string> &argv)
	{
		int argc = argv.size();

		// picosel from inputfile select col where col >= 1 and col < 2
		
		size_t i = 1; 
		while (i < argc) {
			std:: string argi = argv[i];
			if (argi == "-o") {
				if (! (i + 1 < argc)) {
					std:: cerr << "error: option -o requires an argument" << std:: endl;
					return 1;
				}
				outputFile = argv[i + 1];
				i += 2;
			}
			else if (argi == "from" || argi == "FROM") {
				if (! (i + 1 < argc)) {
					std:: cerr << "error: 'from' requires an argument" << std:: endl;
					return 1;
				}
				fromFile = argv[i + 1];
				i += 2;
			}
			else if (argi == "select" || argi == "SELECT") {
				if (! (i + 1 < argc)) {
					std:: cerr << "error: 'select' requires an argument" << std:: endl;
					return 1;
				}
				selectCol = argv[i + 1];
				i += 2;
			}
			else if (argi == "where" || argi == "WHERE" || argi == "order" || argi == "ORDER") {
				break; // for i
			}
			else if (argi == "-b") {
				if (! (i + 1 < argc)) {
					std:: cerr << "error: option -b requires an argument" << std:: endl;
					return 1;
				}
				binaryOutputFactor = boost::lexical_cast<long long>(argv[i + 1]);
				i += 2;
			}
			else {
				std:: cerr << "error: invalid command-line argument" << std:: endl;
				return 1;
			}
		}

		if (fromFile.empty()) {
			std:: cerr << "error: no input file is given" << std:: endl;
			return 1;
		}

		if (selectCol.empty()) {
			std:: cerr << "error: no column is selected" << std:: endl;
			return 1;
		}
		
		if (! (i < argc)) {
			return 0;
		}

		std:: string argi = argv[i];
		if (argi == "ORDER" || argi == "order") {
			// order by  column [asc|desc]
			// i     i+1 i+2    i+3
			if (! (i + 2 < argc)) {
				std:: cerr << "error: invaid expression" << std:: endl;
				return 1;
			}
			argi = argv[i + 1];
			if (! (argi == "BY" || argi == "by")) {
				std:: cerr << "error: invaid expression" << std:: endl;
				return 1;
			}
			std:: string col = argv[i + 2];
			if (i + 3 < argc) {
				argi = argv[i + 3];
				if (argi == "ASC" || argi == "asc") {
					orderByAsc = col;
				}
				else if (argi == "DESC" || argi == "desc") {
					orderByDesc = col;
				}
				else {
					std:: cerr << "error: either 'asc' or 'desc' is expected" << std:: endl;
					return 1;
				}
				i += 3;
				if (i < argc) {
					std:: cerr << "error: too many parameters" << std:: endl;
					return 1;
				}
			}
			else {
				orderByAsc = col;
			}
		}
		else if (argi == "where" || argi == "WHERE") {
			++i;
			
			std::string s;
			if (i < argc && (s = argv[i]).find(' ') != std::string::npos) {
				if (s.length() >= 2 && s[0] == '\"' && s[s.length() - 1] == '\"') {
					s = s.substr(1, s.length() - 2); // remove the quote's.
				}
				std::vector<std::string> exprArgs;
				boost::algorithm::split(exprArgs, s, boost::algorithm::is_space());
				size_t index = 0;
				if (! scanExpr(&conditions, exprArgs, &index) || index != exprArgs.size()) {
					return 1;
				}
			}
			else {
				if (! scanExpr(&conditions, argv, &i)) {
					return 1;
				}
			}
		}
		else {
			std:: cerr << "error: either 'where' or 'order' is expected" << std:: endl;
			return 1;
		}

		if (binaryOutputFactor != 0) {
			if (outputFile.empty()) {
				std:: cerr << "error: option -b requires option -o" << std:: endl;
				return 1;
			}
		}

		return 0;
	}

	static bool scanExpr(std::vector<Condition> *pConditions, const std::vector<std::string> &argv, size_t *pIndex) 
	{
		std::vector<Condition> &conditions = *pConditions;
		size_t argc = argv.size();
		size_t &i = *pIndex;

		OperatorTable opeTbl;
		while (i < argc) {
			std:: string col = argv[i];
			++i;
			if (! (i + 2 <= argc)) {
				std:: cerr << "error: invalid expression" << std:: endl;
				return false;
			}
			
			std:: string opeStr = argv[i];
			Operator ope = opeTbl.toOperator(opeStr);
			if (ope == OP_NULL) {
				std:: cerr << "error: unknown operator '" << opeStr << "'" << std:: endl;
				return false;
			}
			++i;
			
			long double num;
			std:: string numStr = argv[i];
			if (! str_to_ld(&num, numStr)) {
				std:: cerr << "error: invalid number literal '" << numStr << "'" << std:: endl;
				return false;
			}
			++i;

			Condition cond;
			cond.col = col;
			cond.ope = ope;
			cond.num = num;
			conditions.push_back(cond);

			if (i + 1 < argc) {
				std:: string s = argv[i];
				if (! (s == "and" || s == "AND")) {
					std:: cerr << "error: invalid token '" << s << "'" << std:: endl;
					return false;
				}
				++i;
			}
		}
		return true;
	}

	static bool str_to_ld(long double *pNum, const std:: string &numStr) 
	{
		char *p;
		long longValue = strtol(numStr.c_str(), &p, 10);
		if (p - numStr.c_str() == numStr.length()) {
			*pNum = longValue;
			return true;
		}
		else {
			double doubleValue = strtod(numStr.c_str(), &p);
			if (p - numStr.c_str() == numStr.length()) {
				*pNum = doubleValue;
				return true;
			}
		}
		return false;
	}
};

#if ! defined BUILD_PICOSELLIB

int main(int argc, char *argv[])
{
	::setlocale(LC_ALL, "");

	return Main().main(argc, argv);
}

#else

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <jni.h>

#include "../GemX/utility_Picosel.h"

#ifdef _MSC_VER
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

JNIEXPORT jint JNICALL Java_utility_Picosel_invokePicosel
  (JNIEnv *env, jclass, jstring tableFile0, jstring outputFile0, jstring column0, jobjectArray expressions0)
{
	std:: string outputFile;
	{
		const char *outputFileStr = env->GetStringUTFChars(outputFile0, NULL);
		outputFile = outputFileStr;
		env->ReleaseStringUTFChars(outputFile0, outputFileStr);
	}

	std:: string tableFile;
	{
		const char *tableFileStr = env->GetStringUTFChars(tableFile0, NULL);
		tableFile = tableFileStr;
		env->ReleaseStringUTFChars(tableFile0, tableFileStr);
	}

	std:: string column;
	{
		const char *columnStr = env->GetStringUTFChars(column0, NULL);
		column = columnStr;
		env->ReleaseStringUTFChars(column0, columnStr);
	}

	std:: vector<std:: string> expressions;
	{
		size_t exprSize = env->GetArrayLength(expressions0);
		for (size_t i = 0; i < exprSize; ++i) {
			jobject obj = env->GetObjectArrayElement(expressions0, i);
			jstring str = (jstring)obj;
			const char *s = env->GetStringUTFChars(str, NULL);
			expressions.push_back(std:: string(s));
			env->ReleaseStringUTFChars(str, s);
		}
	}

	std:: vector<const char *> argv;
	argv.push_back("picosel"); // dummy argv[0]
	argv.push_back("-o");
	argv.push_back(outputFile.c_str());
	argv.push_back("from");
	argv.push_back(tableFile.c_str());
	argv.push_back("select");
	argv.push_back(column.c_str());
	argv.push_back("where");
	for (size_t i = 0; i < expressions.size(); ++i) {
		argv.push_back(expressions[i].c_str());
	}

	return (jint)Main().main(argv.size(), &argv[0]);
}

#endif
