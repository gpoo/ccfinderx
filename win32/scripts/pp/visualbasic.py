#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import pp.preprocessor as pp
import easytorq
import sys

def getname():
    return "visualbasic"

def getversion():
    return (2, 0, 0, 0)

class VisualbasicPreprocessor(pp.Base):
    def __init__(self):
        self.pat = None
        self.fmt = None
    
    def getoptiondescription(self):
        return "no options."
    
    def tonormalizedoptionstring(self, optionStr):
        if optionStr not in ( None, '', 'default' ):
            raise pp.InvalidOptionError, "invalid option: " + optionStr
        return 'default'
    
    def setoptions(self, optionStr):
        if optionStr not in ( None, '', 'default' ):
            raise pp.InvalidOptionError, "invalid option: " + optionStr
        patternStr = """TEXT scan= +(xcep(eof) any) | insert(eol) eof; // ensure a line terminates by eol

TEXT match= *(
        (macro_line <- "#" *(xcep(eof | eol) any)) eol
        | ?(label <- +("&(a-z);" | "&(A-Z)" | "&(0-9);" | "_") ":") *(xcep(eof | eol) any) eol
    ) 
    *(xcep(eof) any) eof;

TEXT scan= (comment <- "'" *(xcep(eof | eol) any)) 
    | (comment <- "rem" (" " | "&t;") *(xcep(eof | eol) any))
    | (null <- ((" " | "&t;") "_" *(" " | "&t;") eol)); // Continuation line

TEXT scan=
    preq("&(a-z);" | "&(A-Z);") (
        (r_AddHandler <- "AddHandler" | "addhandler" | "ADDHANDLER")
        | (r_AddressOf <- "AddressOf" | "addressof" | "ADDRESSOF")
        | (r_Alias <- "Alias" | "alias" | "ALIAS")
        | (r_AndAlso <- "AndAlso" | "andalso" | "ANDALSO")
        | (r_And <- "And" | "and" | "AND")
        | (r_Ansi <- "Ansi" | "ansi" | "ANSI")
        | (r_Assembly <- "Assembly" | "assembly" | "ASSEMBLY")
        | (r_As <- "As" | "as" | "AS")
        | (r_Auto <- "Auto" | "auto" | "AUTO")
        | (m_BeginProperty <- "BeginProperty" | "beginproperty" | "BEGINPROPERTY")
        | (r_Begin <- "Begin" | "begin" | "BEGIN")
        | (r_Boolean <- "Boolean" | "boolean" | "BOOLEAN")
        | (r_ByRef <- "ByRef" | "byref" | "BYREF")
        | (r_Byte <- "Byte" | "byte" | "BYTE")
        | (r_ByVal <- "ByVal" | "byval" | "BYVAL")
        | (r_Call <- "Call" | "call" | "CALL")
        | (r_Case <- "Case" | "case" | "CASE")
        | (r_Catch <- "Catch" | "catch" | "CATCH")
        | (r_CBool <- "CBool" | "cbool" | "CBOOL")
        | (r_CByte <- "CByte" | "cbyte" | "CBYTE")
        | (r_CChar <- "CChar" | "cchar" | "CCHAR")
        | (r_CDate <- "CDate" | "cdate" | "CDATE")
        | (r_CDec <- "CDec" | "cdec" | "CDEC")
        | (r_CDbl <- "CDbl" | "cdbl" | "CDBL")
        | (r_Char <- "Char" | "char" | "CHAR")
        | (r_CInt <- "CInt" | "cint" | "CINT")
        | (r_Class <- "Class" | "class" | "CLASS")
        | (r_CLng <- "CLng" | "clng" | "CLNG")
        | (r_CObj <- "CObj" | "cobj" | "COBJ")
        | (r_Const <- "Const" | "const" | "CONST")
        | (r_CShort <- "CShort" | "cshort" | "CSHORT")
        | (r_CSng <- "CSng" | "csng" | "CSNG")
        | (r_CStr <- "CStr" | "cstr" | "CSTR")
        | (r_CType <- "CType" | "ctype" | "CTYPE")
        | (r_Date <- "Date" | "date" | "DATE")
        | (r_Decimal <- "Decimal" | "decimal" | "DECIMAL")
        | (r_Declare <- "Declare" | "declare" | "DECLARE")
        | (r_Default <- "Default" | "default" | "DEFAULT")
        | (r_Delegate <- "Delegate" | "delegate" | "DELEGATE")
        | (r_Dim <- "Dim" | "dim" | "DIM")
        | (r_DirectCast <- "DirectCast" | "directcast" | "DIRECTCAST")
        | (r_Double <- "Double" | "double" | "DOUBLE")
        | (r_Do <- "Do" | "do" | "DO")
        | (r_Each <- "Each" | "each" | "EACH")
        | (r_ElseIf <- "ElseIf" | "elseif" | "ELSEIF")
        | (r_Else <- "Else" | "else" | "ELSE")
        | (m_EndProperty <- "EndProperty" | "endproperty" | "ENDPROPERTY")
        | (r_End <- "End" | "end" | "END")
        | (r_Enum <- "Enum" | "enum" | "ENUM")
        | (r_Erase <- "Erase" | "erase" | "ERASE")
        | (r_Error <- "Error" | "error" | "ERROR")
        | (r_Event <- "Event" | "event" | "EVENT")
        | (r_Exit <- "Exit" | "exit" | "EXIT")
        | (r_False <- "False" | "false" | "FALSE")
        | (r_Finally <- "Finally" | "finally" | "FINALLY")
        | (r_For <- "For" | "for" | "FOR")
        | (r_Friend <- "Friend" | "friend" | "FRIEND")
        | (r_Function <- "Function" | "function" | "FUNCTION")
        | (r_GetType <- "GetType" | "gettype" | "GETTYPE")
        | (r_Get <- "Get" | "get" | "GET")
        | (r_GoSub <- "GoSub" | "gosub" | "GOSUB")
        | (r_GoTo <- "GoTo" | "goto" | "GOTO")
        | (r_Handles <- "Handles" | "handles" | "HANDLES")
        | (r_If <- "If" | "if" | "IF")
        | (r_Implements <- "Implements" | "implements" | "IMPLEMENTS")
        | (r_Imports <- "Imports" | "imports" | "IMPORTS")
        | (r_Inherits <- "Inherits" | "inherits" | "INHERITS")
        | (r_Integer <- "Integer" | "integer" | "INTEGER")
        | (r_Interface <- "Interface" | "interface" | "INTERFACE")
        | (r_In <- "In" | "in" | "IN")
        | (m_IsArray <- "IsArray" | "isarray" | "ISARRAY")
        | (m_IsDate <- "IsDate" | "isdate" | "ISDATE")
        | (m_IsEmpty <- "IsEmpty" | "isempty" | "ISEMPTY")
        | (m_IsNull <- "IsNull" | "isnull" | "ISNULL")
        | (m_IsNumeric <- "IsNumeric" | "isnumeric" | "ISNUMERIC")
        | (m_IsObject <- "IsObject" | "isobject" | "ISOBJECT")
        | (r_Is <- "Is" | "is" | "IS")
        | (r_Let <- "Let" | "let" | "LET")
        | (r_Lib <- "Lib" | "lib" | "LIB")
        | (r_Like <- "Like" | "like" | "LIKE")
        | (r_Long <- "Long" | "long" | "LONG")
        | (r_Loop <- "Loop" | "loop" | "LOOP")
        // | (r_Me <- "Me" | "me" | "ME")
        | (r_Module <- "Module" | "module" | "MODULE")
        | (r_Mod <- "Mod" | "mod" | "MOD")
        | (r_MustInherit <- "MustInherit" | "mustinherit" | "MUSTINHERIT")
        | (r_MustOverride <- "MustOverride" | "mustoverride" | "MUSTOVERRIDE")
        | (r_MyBase <- "MyBase" | "mybase" | "MYBASE")
        | (r_MyClass <- "MyClass" | "myclass" | "MYCLASS")
        | (r_Namespace <- "Namespace" | "namespace" | "NAMESPACE")
        | (r_New <- "New" | "new" | "NEW")
        | (r_Next <- "Next" | "next" | "NEXT")
        | (r_Nothing <- "Nothing" | "nothing" | "NOTHING")
        | (r_NotInheritable <- "NotInheritable" | "notinheritable" | "NOTINHERITABLE")
        | (r_NotOverridable <- "NotOverridable" | "notoverridable" | "NOTOVERRIDABLE")
        | (r_Not <- "Not" | "not" | "NOT")
        | (r_Object <- "Object" | "object" | "OBJECT")
        | (r_On <- "On" | "on" | "ON")
        | (r_Optional <- "Optional" | "optional" | "OPTIONAL")
        | (r_Option <- "Option" | "option" | "OPTION")
        | (r_OrElse <- "OrElse" | "orelse" | "ORELSE")
        | (r_Or <- "Or" | "or" | "OR")
        | (r_Overloads <- "Overloads" | "overloads" | "OVERLOADS")
        | (r_Overridable <- "Overridable" | "overridable" | "OVERRIDABLE")
        | (r_Overrides <- "Overrides" | "overrides" | "OVERRIDES")
        | (r_ParamArray <- "ParamArray" | "paramarray" | "PARAMARRAY")
        | (r_Preserve <- "Preserve" | "preserve" | "PRESERVE")
        | (r_Private <- "Private" | "private" | "PRIVATE")
        | (r_Property <- "Property" | "property" | "PROPERTY")
        | (r_Protected <- "Protected" | "protected" | "PROTECTED")
        | (r_Public <- "Public" | "public" | "PUBLIC")
        | (r_RaiseEvent <- "RaiseEvent" | "raiseevent" | "RAISEEVENT")
        | (r_ReadOnly <- "ReadOnly" | "readonly" | "READONLY")
        | (r_ReDim <- "ReDim" | "redim" | "REDIM")
        | (r_REM <- "REM" | "rem" | "REM")
        | (r_RemoveHandler <- "RemoveHandler" | "removehandler" | "REMOVEHANDLER")
        | (r_Resume <- "Resume" | "resume" | "RESUME")
        | (r_Return <- "Return" | "return" | "RETURN")
        | (r_Select <- "Select" | "select" | "SELECT")
        | (r_Set <- "Set" | "set" | "SET")
        | (r_Shadows <- "Shadows" | "shadows" | "SHADOWS")
        | (r_Shared <- "Shared" | "shared" | "SHARED")
        | (r_Short <- "Short" | "short" | "SHORT")
        | (r_Single <- "Single" | "single" | "SINGLE")
        | (r_Static <- "Static" | "static" | "STATIC")
        | (r_Step <- "Step" | "step" | "STEP")
        | (r_Stop <- "Stop" | "stop" | "STOP")
        | (r_String <- "String" | "string" | "STRING")
        | (r_Structure <- "Structure" | "structure" | "STRUCTURE")
        | (r_Sub <- "Sub" | "sub" | "SUB")
        | (r_SyncLock <- "SyncLock" | "synclock" | "SYNCLOCK")
        | (r_Then <- "Then" | "then" | "THEN")
        | (r_Throw <- "Throw" | "throw" | "THROW")
        | (r_To <- "To" | "to" | "TO")
        | (r_True <- "True" | "true" | "TRUE")
        | (r_Try <- "Try" | "try" | "TRY")
        | (r_TypeOf <- "TypeOf" | "typeof" | "TYPEOF")
        | (r_Type <- "Type" | "type" | "TYPE")
        | (r_Unicode <- "Unicode" | "unicode" | "UNICODE")
        | (r_Until <- "Until" | "until" | "UNTIL")
        | (r_Variant <- "Variant" | "variant" | "VARIANT")
        | (r_Wend <- "Wend" | "wend" | "WEND")
        | (r_When <- "When" | "when" | "WHEN")
        | (r_While <- "While" | "while" | "WHILE")
        | (r_WithEvents <- "WithEvents" | "withevents" | "WITHEVENTS")
        | (r_With <- "With" | "with" | "WITH")
        | (r_WriteOnly <- "WriteOnly" | "writeonly" | "WRITEONLY")
        | (r_Xor <- "Xor" | "xor" | "XOR")
        | (r_GoSub <- "GoSub" | "gosub" | "GOSUB")
        | (r_Let <- "Let" | "let" | "LET")
        | (r_Variant <- "Variant" | "variant" | "VARIANT")
        | (m_MsgBox <- "MsgBox" | "msgbox" | "MSGBOX")
        | (m_Iif <- "Iif" | "iif" | "IIF")
        | (m_InputBox <- "InputBox" | "inputbox" | "INPUTBOX")
    ) xcep("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);")
    | (word <- ("&(a-z);" | "&(A-Z);" | "_") *("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);"))
    | (word <- "[" ("&(a-z);" | "&(A-Z);" | "_") *("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);") "]")
    | (l_char <- "&quot;" *("&quot;&quot;" | xcep("&quot;" | eol) any) "&quot;") ("C" | "c")
    | (l_string <- "&quot;" *("&quot;&quot;" | xcep("&quot;" | eol) any) "&quot;")
    | (l_string <- "#" *(" " | "&t;" | "&(0-9);" | "/" | ":" | "AM" | "am" | "PM" | "pm") "#") // date
    | (l_float <- (
            +"&(0-9);" "." *"&(0-9);" ?(("e" | "E") ?("-" | "+") +"&(0-9);") ?("F" | "f" | "R" | "r" | "D" | "d") 
            | +"&(0-9);" ("e" | "E") ?("-" | "+") +"&(0-9);") ?("F" | "f" | "R" | "r" | "D" | "d") 
            | +"&(0-9);" ("F" | "f" | "R" | "r" | "D" | "d")
    )
    | (l_int <- (
        "&amp;" ("H" | "h") +("&(0-9);" | "&(a-f);" | "&(A-F);") 
        | "&amp;" ("O" | "o") +"&(0-7);"
        | +"&(0-9);") 
        ?("S" | "s" | "I" | "i" | "L"| "l")
    )
    | (LP <- "(") | (RP <- ")") 
    | (op_plus_eq <- "+=")
    | (op_minus_eq <- "-=")
    | (op_mul_eq <- "*=")
    | (op_div_eq <- "/=")
    | (op_intdiv_eq <- "&slash;=")
    | (op_pow_eq <- "^=")
    | (op_append_eq <- "&=")
    | (op_lshift <- "<<")
    | (op_rshift <- ">>")
    | (op_not_eq <- "<>")
    | (op_ge <- ">=")
    | (op_le <- "<=")
    | (op_gt <- ">")
    | (op_lt <- "<")
    | (equal <- "=")
    | (exclamation <- "!")
    | (comma <- ",")
    | (dot <- ".")
    | (colon <- ":")
    | (plus <- "+")
    | (minus <- "-")
    | (op_mult <- "*")
    | (op_div <- "/")
    | (op_intdiv <- "&bslash;")
    | (amp <- "&amp;")
    | (op_power <- "^");

TEXT scan= (null <- macro_line | comment | label | " " | "&t;")
    | (null <- m_BeginProperty *(xcep(m_EndProperty) any) m_EndProperty)
    | insert(statement_terminator) (null <- eol);

TEXT scan= (r_EXIT_LOOP <- r_Exit (r_Do | r_While | r_Loop))
    | (r_Exit_For <- r_Exit r_for)
    | (r_Exit_Function <- r_Exit r_Function)
    | (r_Exit_Property <- r_Exit r_Property)
    | (r_Exit_Sub <- r_Exit r_Sub)
    | (r_Exit_Try <- r_Exit r_Try)
    | (r_On_Error <- r_On r_Error)
    | (r_Resume_Next <- r_Resume r_Next);

TEXT scan= (id <- ?(dot | exclamation) word *((dot | exclamation) word) ?("%" | amp | "@" | exclamation | "#" | "$"))
    | (statement_terminator <- ":")
    | (l_bool <- r_True | r_False)
    | (r_Integer <- r_Long | r_Byte) // type unification
    | (r_Double <- r_Single) // type unification
    | (null <- r_Then | r_Call | r_Let | r_ByRef | r_Dim)
    | (null <- r_Public | r_Private | r_Protected ?r_Friend | r_Friend | r_Overloads | r_Overrides | r_Overridable);

TEXT scan= statement_terminator (null <- +statement_terminator);

TEXT scan= (null <- r_Declare *(xcep(statement_terminator| eof) any) statement_terminator)
    | (null <- r_Interface *(xcep(r_End) any | r_End xcep(r_Interface) any) r_End r_Interface)
    | (null <- r_WithEvents id r_As id statement_terminator);

TEXT scan= 
    // note: the following rule will not identify blocks around if statement, 
    //   becase I can not invent a parsing rule which support both "if...end if" statement 
    //   and "if statement" enclosed in a line, that appears without "end if".
    (block <- r_Begin *^ r_End xcep(r_Class | r_Function | r_Get | r_Interface | r_Module 
        | r_Namespace | r_Property | r_Set | r_Structure | r_Sub | r_Select | r_Type | r_Try | r_With | r_If))
    | (def_block <- r_Class ?(id *statement_terminator ?(null <- r_Inherits id)) (block <- *^) r_End r_Class)
    | (def_block <- r_Structure ?(id *statement_terminator ?(null <- r_Inherits id)) (block <- *^) r_End r_Structure)
    | (block <- r_Namespace ?(id *statement_terminator (block <- *^)) r_End r_Namespace)
    | (block <- r_Do (r_While | r_Until) *^ r_Loop)
    | (block <- r_Do *^ r_Loop ?(r_While  | r_Until))
    | (block <- insert(r_Do) r_While *^ (r_Loop <- r_End r_While | r_Wend)) // While ... End While, While ... Wend --> Do While ... Loop
    | (block <- r_For ?r_Each *^ r_Next)
    | (def_block <- r_Function *^ r_End r_Function)
    | (def_block <- r_Get *^ r_End r_Get)
    | (def_block <- r_Interface *^ r_End r_Interface)
    | (def_block <- r_Module *^ r_End r_Module)
    | (def_block <- r_Property ?(r_Get | r_Let | r_Set) *^ r_End r_Property)
    | (def_block <- r_Set preq(LP) *^ r_End r_Set)
    | (def_block <- r_Sub *^ r_End r_Sub)
    | (block <- r_Select r_Case *^ *(r_Case ?r_Else (block <- *^)) r_End r_Select)
    | (def_block <- r_Type *^ r_End r_Type)
    | (block <- r_Try (block <- *^) *(r_Catch (block <- *^)) r_End r_Try)
    | (block <- r_With ?(null <- id) (block <- *^) r_End r_With)
    | r_EXIT_LOOP | r_Exit_For | r_Exit_Function | r_Exit_Property | r_Exit_Sub | r_Exit_Try
    | r_Exit xcep(eof) any
    | r_End r_If
    | +xcep(eof | r_End | r_Class | r_Do | r_Loop | r_For | r_Next | r_Function | r_Get | r_Interface | r_Module | r_Namespace | 
        r_Property | r_Set | r_Structure | r_Sub | r_Select | r_Type | r_Try | r_With) any
    | xcep(eof | r_End | r_Loop | r_Next) any;

// insert tokens for control-flow complexity counter
TEXT scan= (r_End (r_If | r_Select | r_Loop | r_While))
    | (r_If | r_Select) insert(c_cond) | r_For insert(c_loop) | (r_Do | r_Loop) (r_While | r_Until) insert(c_loop) | r_While insert(c_loop)
    | (block scan ^) | (def_block scan ^); // recurse into block

TEXT scan= ((block scan ^) | (def_block scan ^)) *(null <- statement_terminator);
"""
        self.pat = easytorq.Pattern(patternStr)
        
        fmt = easytorq.CngFormatter()
        
        # parameter by default
        fmt.addreplace('id', 'id|%s')
        
        # non parameter by default
        fmt.addreplace('l_bool', 'l_bool|%s')
        fmt.addreplace('l_char', 'l_char|%s')
        fmt.addreplace('l_int', 'l_int|%s')
        fmt.addreplace('l_float', 'l_float|%s')
        fmt.addreplace('l_string', 'l_string|%s')
        
        fmt.addflatten('word')
        fmt.addreplace('LP', '(paren')
        fmt.addreplace('RP', ')paren')
        fmt.addformat('block', '(block', ')block')
        fmt.addformat('def_block', '(def_block', ')def_block')
        fmt.addreplace('statement_terminator', 'suffix:colon')
        self.fmt = fmt
    
    def getnormalizedoptionstring(self):
        return 'default'
    
    def getdefaultparameterizing(self):
        return { "id" : pp.Param.P_MATCH }
    
    def parse(self, sourceCodeStrInUtf8):
        if self.pat == None:
            self.setoptions(None)
        
        t = easytorq.Tree(sourceCodeStrInUtf8)
        self.pat.apply(t)
        s = self.fmt.format(t)
        return s
    
def getpreprocessor():
    return VisualbasicPreprocessor()

if __name__ == '__main__':
    cnv = easytorq.ICUConverter()
    cnv.setencoding("char")
    
    f = file(sys.argv[1], "rb")
    str = f.read()
    f.close()
    
    strUtf8 = cnv.decode(str)
    
    prep = preprocessor()
    prep.setoptions("")
    s = prep.parse(strUtf8)
    sys.stdout.write(s)
    
