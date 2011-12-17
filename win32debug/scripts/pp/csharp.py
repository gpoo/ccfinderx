#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import pp.preprocessor as pp
import easytorq
import sys

def default_option_values():
    return { 'k' : False }

def getname():
    return "csharp"

def getversion():
    return (2, 0, 0, 0)

class CSharpPreprocessor(pp.Base):
    def __topm(self, value):
        if value: 
            return "+"
        else: 
            return "-"
        
    def __tov(self, pm):
        assert pm in ( "+", "-" )
        return pm == "+"

    def __init__(self):
        self.pat = None
        self.fmt = None
        self.optionValues = default_option_values()
        self.__defaultOptionValues = default_option_values()
        self.__defaultOptionString = "".join("%s%s" % (name, self.__topm(value)) for name, value in sorted(self.__defaultOptionValues.iteritems()))
    
    def getoptiondescription(self):
        return r"""{
    "-r k" : [ "checkbox", false, "k+", "k-",
        "keep declarations or statements." ]
}
"""
    
    def tonormalizedoptionstring(self, optionStr):
        if optionStr == 'default':
            optionStr = ''
        optionValues = self.__defaultOptionValues.copy()
        if optionStr != None and optionStr != '':
            for i in range(0, len(optionStr), 2):
                name = optionStr[i]
                value = optionStr[i + 1]
                if name not in optionValues:
                    raise pp.InvalidOptionError, "invalid option: " + optionStr
                optionValues[name] = self.__tov(value)
                if value == '+':
                    optionValues[name] = True
                elif value == '-':
                    optionValues[name] = False
                else:
                    raise pp.InvalidOptionError, "invalid option: " + optionStr
        s = "".join("%s%s" % (name, self.__topm(value)) for name, value in sorted(optionValues.iteritems()))
        if s == self.__defaultOptionString:
            return 'default'
        else:
            return s
    
    def setoptions(self, optionStr):
        optionStr = self.tonormalizedoptionstring(optionStr)
        if optionStr == 'default':
            optionValues = self.__defaultOptionValues
        else:
            optionValues = self.optionValues.copy()
            if optionStr != None and optionStr != '':
                for i in range(0, len(optionStr), 2):
                    name = optionStr[i]
                    value = optionStr[i + 1]
                    assert name in optionValues
                    optionValues[name] = self.__tov(value)
        self.optionValues = optionValues
        
        if optionValues['k']:
            switch_statement_rule = """
    | r_case +(xcep(colon | semicolon | eof) any) colon
    | r_default colon
    | r_switch (block scan ^)
"""
            simple_statement_removal_rule = ""
        else:
            switch_statement_rule = """
    | +((r_case (id | l_bool | l_char | l_int) | r_default) colon) ((block scan ^) ?(null <- r_break semicolon) | (block <- (insert(LB) *(xcep(r_break | r_case | r_default) ^) insert(RB))) ?(null <- r_break semicolon)) // enclose each case clause by block
    | r_switch (block scan ^)
"""
            simple_statement_removal_rule = """
TEXT scan= (null <- simple_statement)
    | r_namespace id (block scan ^)
    | (r_class | r_struct) id (null <- ?(colon *(xcep(semicolon | block) any))) (block scan ^); // recurse into top level of class definition
"""
        
        patternStr = """TEXT scan= 
    preq("&(a-z);") (
        (r_abstract <- "abstract")
        (r_alias <- "alias")
        | (r_as <- "as")
        // | (r_base <- "base") // keyword "base" is treated as an identifier
        | (r_bool <- "bool")
        | (r_break <- "break")
        | (r_byte <- "byte")
        | (r_case <- "case")
        | (r_catch <- "catch")
        | (r_char <- "char")
        | (r_checked <- "checked")
        | (r_class <- "class")
        | (r_const <- "const")
        | (r_continue <- "continue")
        | (r_decimal <- "decimal")
        | (r_default <- "default")
        | (r_delegate <- "delegate")
        | (r_double <- "double")
        | (r_do <- "do")
        | (r_else <- "else")
        | (r_enum <- "enum")
        | (r_event <- "event")
        | (r_explicit <- "explicit")
        | (r_extern <- "extern")
        | (r_false <- "false")
        | (r_finally <- "finally")
        | (r_fixed <- "fixed")
        | (r_float <- "float")
        | (r_foreach <- "foreach")
        | (r_for <- "for")
        | (r_get <- "get")
        | (r_goto <- "goto")
        | (r_if <- "if")
        | (r_implicit <- "implicit")
        | (r_interface <- "interface")
        | (r_internal <- "internal")
        | (r_int <- "int") | (r_in <- "in")
        | (r_is <- "is")
        | (r_lock <- "lock")
        | (r_long <- "long")
        | (r_namespace <- "namespace")
        | (r_new <- "new")
        | (r_null <- "null")
        //| (r_object <- "object") // keyword "object" is treated as an identifier
        | (r_operator <- "operator")
        | (r_out <- "out")
        | (r_override <- "override")
        | (r_params <- "params")
        | (r_partial <- "partial")
        | (r_private <- "private")
        | (r_protected <- "protected")
        | (r_public <- "public")
        | (r_readonly <- "readonly")
        | (r_ref <- "ref")
        | (r_return <- "return")
        | (r_sbyte <- "sbyte")
        | (r_sealed <- "sealed")
        | (r_set <- "set")
        | (r_short <- "short")
        | (r_sizeof <- "sizeof")
        | (r_stackalloc <- "stackalloc")
        | (r_static <- "static")
        | (r_string <- "string")
        | (r_struct <- "struct")
        | (r_switch <- "switch")
        // | (r_this <- "this") // keyword "this" is treated as an identifier
        | (r_throw <- "throw")
        | (r_true <- "true")
        | (r_try <- "try")
        | (r_typeof <- "typeof")
        | (r_uint <- "uint")
        | (r_ulong <- "ulong")
        | (r_unchecked <- "unchecked")
        | (r_unsafe <- "unsafe")
        | (r_ushort <- "ushort")
        | (r_using <- "using")
        // | (r_value <- "value") // keyword "value" is treated as an identifier
        | (r_virtual <- "virtual")
        | (r_void <- "void")
        | (r_volatile <- "volatile")
        | (r_while <- "while")
        | (r_yield <- "yield")
    ) xcep("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);")
    | preq("&(A-Z);") (
        (m_Clone <- "Clone")
        | (m_CompareTo <- "CompareTo")
        | (m_Dispose <- "Dispose")
        | (m_Equals <- "Equals")
        | (m_GetHashCode <- "GetHashCode")
        | (m_GetType <- "GetType")
        | (m_InitializeComponent <- "InitializeComponent")
        | (m_Nullable <- ?"System." "Nullable")
        | (m_ReferenceEquals <- "ReferenceEquals")
        | (m_ToString <- "ToString")
        | (r_object <- "System.Object" | "Object")
        | (r_string <- "System.String" | "String")
        | (r_char <- "System.Char" | "Char")
        | (r_sbyte <- "System.SByte" | "SByte")
        | (r_short <- "System.Int16" | "Int16")
        | (r_ushort <- "System.UInt16" | "UInt16")
        | (r_int <- "System.Int32" | "Int32")
        | (r_uint <- "System.UInt32" | "UInt32")
        | (r_long <- "System.Int64" | "Int64")
        | (r_ulong <- "System.UInt64" | "UInt64")
        | (r_float <- "System.Single" | "Single")
        | (r_double <- "System.Double" | "Double")
        | (r_bool <- "System.Boolean" | "Boolean")
        | (r_decimal <- "System.Decimal" | "Decimal")
    ) xcep("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);")
    | (word <- ?"@" ("&(a-z);" | "&(A-Z);" | "_") *("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);"))
    | (multiline_comment <- "/*" *(+"*" (xcep("/") any) | xcep("*") any) +"*" "/")
    | (singleline_comment <- "//" *(xcep(eol) any))
    | (l_string <- "@" "&quot;" *("&quot;" "&quot;" | xcep("&quot;" | eof) any) "&quot;") // berbatim string
    | (l_string <- "&quot;" *("&bslash;" any | xcep("&quot;" | eol) any) "&quot;")
    | (l_char <- "&squot;" *("&bslash;" any | xcep("&squot;" | "&quot;" | eol) any) "&squot;")
    | (l_float <- (
            +"&(0-9);" "." *"&(0-9);" ?(("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "F" | "d" | "D" | "m" | "M") 
            | +"&(0-9);" ("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "F" | "d" | "D" | "m" | "M")
            | +"&(0-9);" ("f" | "F" | "d" | "D" | "m" | "M")
    )
    | (l_int <- (("0x" | "0X") +("&(0-9);" | "&(a-f);" | "&(A-F);") | +"&(0-9);") *("l" | "L" | "u" | "U"))
    | (macro_line <- "#" *(xcep(eof | eol) any))
    | (semicolon <- ";")
    | (comma <- ",") 
    | (LB <- "{") | (RB <- "}") 
    | (LP <- "(") | (RP <- ")") 
    | (LK <- "[") | (RK <- "]") 
    // 3 char operators
    | (op_lshift_assign <- "<<=")
    | (op_rshift_assign <- ">>=")
    // 2 char operators
    | (op_lshift <- "<<")
    | (op_rshift <- ">>")
    | (op_increment <- "++")
    | (op_decrement <- "--")
    | (op_le <- "<=")
    | (op_ge <- ">=")
    | (op_eq <- "==")
    | (op_ne <- "!=")
    | (op_add_assign <- "+=")
    | (op_sub_assign <- "-=")
    | (op_mul_assign <- "*=")
    | (op_div_assign <- "/=")
    | (op_mod_assign <- "%%=")
    | (op_and_assign <- "&amp;" "=")
    | (op_xor_assign <- "^=")
    | (op_or_assign <- "|=")
    | (op_logical_and <- "&amp;" "&amp;")
    | (op_logical_or <- "||")
    | (op_lambda <- "=>")
    | (op_namespace_alias_resolution <- "::")
    // single char operators
    | (op_star <- "*") // may mean mul or wildcard
    | (op_div <- "/")
    | (op_mod <- "%%")
    | (op_plus <- "+") // may mean add or sign plus
    | (op_minus <- "-") // may mean sub or sign minus
    | (op_amp <- "&amp;")
    | (op_logical_neg <- "!")
    | (op_complement <- "~")
    | (op_or <- "|")
    | (op_xor <- "^")
    | (op_assign <- "=")
    | (OL <- "<") // may mean less than or template parameter
    | (OG <- ">") // may mean greater than or template parameter
    | (ques <- "?") | (colon <- ":") | (dot <- ".");

TEXT scan= null <- macro_line | multiline_comment | singleline_comment | " " | "&t;" | eol;

TEXT scan= null <- (?r_extern r_alias | r_event | r_delegate xcep(LP | LB | semicolon) any | r_using xcep(LP | semicolon) any) *(xcep(semicolon) any) semicolon;

TEXT match= (null <- +(attribute <- (LK *(xcep(eof | RK) any) RK))) *any | *any;
TEXT scan= (semicolon | RB) (null <- +(attribute <- (LK *(xcep(eof | RK) any) RK))); // remove attribute

TEXT scan= 
    (r_byte <- r_sbyte)
    | (r_int <- r_uint | r_short | r_ushort | r_long | r_ulong) 
    | (r_double <- r_float) 
    | (l_bool <- r_true | r_false);

TEXT scan= xcep(LB | RB | LP | RP) any 
    | (get_set_decl <- LB r_get semicolon ?(r_set semicolon) RB | LB r_set semicolon RB)
    | (block <- LB *^ RB) 
    | (param <- LP *^ RP)
    | (index <- LK *^ RK);

// remove generated code
TEXT scan= (null <- r_void m_InitializeComponent (param match LP RP) block)
    | (null <- r_void m_Dispose (param match LP r_bool (word match "disposing") RP)
        (block match LB r_if (param match LP (word match "disposing") RP) 
            (block match LB r_if (param match LP (word match "components") op_ne r_null RP) 
                (block match LB (word match "components") dot m_Dispose (param match LP RP) semicolon RB)
            RB)
            (word match "base") dot m_Dispose (param match LP (word match "disposing") RP) semicolon
        RB)
    )
    | (block scan ^); // recurse into block

TEXT scan= xcep(OL | OG | block | param) any | (template_param <- OL *^ OG) 
    | (block scan ^) | (param scan ^); // recurse into block and param

TEXT scan= ?(null <- (word match "this") dot) (id <- word *((dot | op_namespace_alias_resolution) word) ?template_param ?ques)
    | (id <- (word match "this"))
    | (string_litral <- l_string +(op_plus l_string))
    | (block scan ^) | (param scan ^); // recurse into block and param

TEXT scan= (null <- m_Nullable OL) id (null <- OG)
    | ques insert(c_cond) // insert tokens for control-flow complexity counter
    | (block scan ^) | (param scan ^); // recurse into block and param

// remove simple delegations; remove empty method definition; remove redundant paren of return statement
TEXT scan=
    (null <- (r_void | r_int | r_long | r_short | r_double | r_float | r_bool | r_char | r_byte | r_decimal | r_string | r_object | id) *(index match LK RK)
         id param ((block match LB ?r_return id dot id param semicolon RB) | (block match LB RB)))
    | r_return (param match (null <- LP) *(xcep(RP) any) (null <- RP)) semicolon
    | op_assign (initialization_block <- preq(block)) (null <- block) semicolon
    | index (initialization_block <- preq(block)) (null <- block)
    | (null <- r_enum id ?(colon any) (initialization_block <- preq(block)) (null <- block))
    | (null <- r_private | r_public | r_protected | r_internal | r_override | r_virtual | r_sealed | r_unsafe | r_static | r_partial)
    | (null <- r_get) (null <- (block match LB r_return (id | l_string | l_char | l_int | l_float | l_bool) semicolon RB)) // simple getter
    | (null <- r_set) (null <- (block match LB id op_assign id semicolon RB)) // simple setter
    | (null <- r_interface id ?(colon id *(comma id)) block)
    | (block scan ^) | (param scan ^); // recurse into block and param

TEXT scan= 
    r_if param ((block scan ^) | (block <- insert(LB) ^ insert(RB))) *(r_else r_if param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))) ?(r_else ((block scan ^) | (block <- insert(LB) ^ insert(RB)))) 
    | r_else ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_while param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | (r_for | r_foreach) param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_do ((block scan ^)| (block <- insert(LB) ^ insert(RB))) r_while param semicolon
    | r_try (block scan ^) *((r_catch ?param | r_finally) (block scan ^))
    | (r_catch ?param | r_finally) (block scan ^)
    %(switch_statement_rule)s
    | *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_foreach | r_do | r_try | r_catch | r_finally | r_switch) any) semicolon
    | (block scan ^); // recurse into block

TEXT scan= 
    r_if param block *(r_else r_if param block) ?(r_else block) 
    | r_else block
    | r_while param block
    | (r_for | r_foreach) param block
    | r_do block r_while param semicolon
    | r_try block *((r_catch ?param | r_finally) block)
    | (r_catch ?param | r_finally) block
    | r_switch param block
    | +(r_using param) block
    | r_delegate ?param block
    | (simple_statement <- *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_foreach | r_do | r_try | r_catch | r_finally | r_switch | r_case | r_default | r_using | r_delegate) any) semicolon)
    | (null <- (r_int | r_long | r_short | r_double | r_float | r_bool | r_char | r_byte | r_decimal | r_string | r_object | id) *(index match LK RK) id (block match LB RB)) // property without any getter/setter
    | (block scan ^); // recurse into block

%(simple_statement_removal_rule)s

// enclose class/method definition by block
TEXT scan= (def_block <- (r_class | r_struct) id (block scan ^))
    | (def_block <- (r_void | r_int | r_long | r_short | r_double | r_float | r_bool | r_char | r_byte | r_decimal | r_string | r_object | id) 
        (
            (id param (block scan ^))
            | (id (block scan ^))
        )
    )
    | (def_block <- (r_get | r_set) (block scan ^))
    | (block scan ^);

// insert tokens for control-flow complexity counter
TEXT scan= (r_if | r_switch) insert(c_cond) | (r_for | r_while | r_foreach) insert(c_loop)
    | id insert(c_func) (param scan ^)
    | (r_get | r_set) insert(c_func) (block scan ^)
    | (def_block scan ^) | (block scan ^) | (param scan ^) | (index scan ^) | (simple_statement scan ^);
""" % (locals())

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
        
        fmt.addflatten('block')
        fmt.addreplace('LB', '(brace')
        fmt.addreplace('RB', ')brace')
        fmt.addflatten('word')
        fmt.addflatten('param')
        fmt.addreplace('LP', '(paren')
        fmt.addreplace('RP', ')paren')
        fmt.addflatten('index')
        fmt.addreplace('LK', '(braket')
        fmt.addreplace('RK', ')braket')
        fmt.addflatten('simple_statement')
        fmt.addreplace('semicolon', 'suffix:semicolon')
        fmt.addreplace('colon', 'suffix:colon')
        fmt.addformat('def_block', '(def_block', ')def_block')
        self.fmt = fmt
    
    def getnormalizedoptionstring(self):
        s = "".join("%s%s" % (name, self.__topm(value)) for name, value in sorted(self.optionValues.iteritems()))
        if s == self.__defaultOptionString:
            return 'default'
        else:
            return s
    
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
    return CSharpPreprocessor()

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
    
