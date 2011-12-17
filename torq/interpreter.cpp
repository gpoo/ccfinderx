#include "interpreter.h"

const boost::int32_t/* code */ Interpreter::cNULL = 0;
const boost::int32_t/* code */ Interpreter::cANY = 1;
const boost::int32_t/* code */ Interpreter::cEOF = 2;
const boost::int32_t/* code */ Interpreter::cEOL = 3;
const boost::int32_t/* code */ Interpreter::cRAW = 4;

boost::shared_ptr<LabelCodeTableBase> LabelCodeTableSingleton::pTheInstance;

#if defined INTERPRETER_USE_DISPATCHTABLE

const Interpreter::pfunc_t Interpreter::dispatchTable[NC_SIZE] = 
{
	/* NC_Nul */ &Interpreter::do_assertionError,
	/* NC_Statements */ &Interpreter::do_assertionError,
	/* NC_Statement */ &Interpreter::do_assertionError,
	/* NC_AssignStatement */ &Interpreter::do_assertionError,
	/* NC_ScanEqStatement */ &Interpreter::do_assertionError,
	/* NC_MatchEqStatement */ &Interpreter::do_assertionError,
	/* NC_Expression */ &Interpreter::do_assertionError,
	/* NC_Pattern */ &Interpreter::do_Pattern,
	/* NC_PackPattern */ &Interpreter::do_PackPattern,
	/* NC_MatchPattern */ &Interpreter::do_MatchPattern,
	/* NC_ScanPattern */ &Interpreter::do_ScanPattern,
	/* NC_OrPattern */ &Interpreter::do_OrPattern,
	/* NC_SequencePattern */ &Interpreter::do_SequencePattern,
	/* NC_RepeatPattern */ &Interpreter::do_RepeatPattern,
	/* NC_AtomPattern */ &Interpreter::do_assertionError,
	/* NC_ParenPattern */ &Interpreter::do_assertionError,
	/* NC_XcepPattern */ &Interpreter::do_XcepPattern,
	/* NC_PreqPattern */ &Interpreter::do_PreqPattern,
	/* NC_LiteralPattern */ &Interpreter::do_LiteralPattern,
	/* NC_GeneratedTokenPattern */ &Interpreter::do_GeneratedTokenPattern,
	/* NC_RecursePattern */ &Interpreter::do_RecursePattern,
	/* NC_InsertPattern */ &Interpreter::do_InsertPattern,
};

#endif // INTEREPRETER_USE_DISPATCHTABLE
