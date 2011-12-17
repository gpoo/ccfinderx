#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import pp.preprocessor as pp
import easytorq
import sys

def default_option_values():
    return { 'd' : True, 'r' : True, 's' : True, 'k' : False }

def getname():
    return "java"

def getversion():
    return (2, 0, 0, 0)

class JavaPreprocessor(pp.Base):
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
    "-r d" : [ "checkbox", true, "d+", "d-",
        "parameterize numerical/booelan literals (e.g. 100 -> l_int, true -> l_bool)." ],
    "-r r" : [ "checkbox", true, "r+", "r-", 
        "neglect interface." ],
    "-r s" : [ "checkbox", true, "s+", "s-",
        "parameterize string literals (e.g. \"abc\" -> l_string, '\\t' -> l_char)." ],
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
    | r_class id *((r_extends | r_implements) id *(comma id)) (block scan ^); // recurse into top level of class definition

"""        
        patternStr = """TEXT scan=
    preq("&(a-z);") (
        (r_abstract <- "abstract")
        | (r_assert <- "assert")
        | (r_boolean <- "boolean")
        | (r_break <- "break")
        | (r_byte <- "byte")
        | (r_case <- "case")
        | (r_catch <- "catch")
        | (m_charAt <- "charAt")
        | (r_char <- "char")
        | (r_class <- "class")
        | (m_clone <- "clone")
        | (m_compareTo <- "compareTo")
        | (r_continue <- "continue")
        | (r_const <- "const")
        | (r_default <- "default")
        | (m_dispose <- "dispose")
        | (r_double <- "double")
        | (r_do <- "do")
        | (r_else <- "else")
        | (r_enum <- "enum")
        | (m_equals <- "equals")
        | (r_extends <- "extends")
        | (r_false <- "false")
        | (r_finally <- "finally")
        | (r_final <- "final")
        | (r_float <- "float")
        | (r_for <- "for")
        | (m_getClass <- "getClass")
        | (m_get <- "get")
        | (r_goto <- "goto")
        | (m_hashCode <- "hashCode")
        | (m_hasNext <- "hasNext")
        | (r_if <- "if")
        | (r_implements <- "implements")
        | (r_import <- "import")
        | (r_instanceof <- "instanceof")
        | (r_interface <- "interface")
        | (r_int <- "int")
        | (m_iterator <- "iterator")
        | (m_length <- "length")
        | (r_long <- "long")
        | (r_native <- "native")
        | (r_new <- "new")
        | (m_next <- "next")
        | (r_null <- "null")
        | (r_package <- "package")
        | (r_private <- "private")
        | (r_protected <- "protected")
        | (r_public <- "public")
        | (r_return <- "return")
        | (m_run <- "run")
        | (r_short <- "short")
        | (m_size <- "size")
        | (r_static <- "static")
        | (r_strictfp <- "strictfp")
        // | (r_super <- "super") // keyword "super" is treated as an identifier
        | (r_switch <- "switch")
        | (r_synchronized <- "synchronized")
        // | (r_this <- "this") // keyword "this" is treated as an identifier
        | (m_toArray <- "toArray")
        | (m_toString <- "toString")
        | (r_throws <- "throws")
        | (r_throw <- "throw")
        | (r_transient <- "transient")
        | (r_true <- "true")
        | (r_try <- "try")
        | (r_void <- "void")
        | (r_volatile <- "volatile")
        | (r_while <- "while")
    ) xcep("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);")
    | (word <- ("&(a-z);" | "&(A-Z);" | "_" | "$") *("&(a-z);" | "&(A-Z);" | "_" | "$" | "&(0-9);"))
    | (multiline_comment <- "/*" *(xcep("*/") any) "*/")
    | (singleline_comment <- "//" *(xcep(eol) any))
    | (l_string <- "&quot;" *("&bslash;" any | xcep("&quot;" | eol) any) "&quot;")
    | (l_char <- "&squot;" *("&bslash;" any | xcep("&squot;" | eol) any) "&squot;")
    | (l_float <- (
            ((+"&(0-9);" "." *"&(0-9);")|(*"&(0-9);" "." +"&(0-9);")) ?(("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "F") // modified by Jan Vlegels, 2007/Apr/23
            | +"&(0-9);" ("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "F") 
            | +"&(0-9);" ("f" | "F")
    )
    | (l_int <- (("0x" | "0X") +("&(0-9);" | "&(a-f);" | "&(A-F);") | +"&(0-9);") *("l" | "L"))
    | (semicolon <- ";")
    | (comma <- ",") 
    | (LB <- "{") | (RB <- "}") 
    | (LP <- "(") | (RP <- ")") 
    | (LK <- "[") | (RK <- "]") 
    // 4 char operator
    | (op_signed_rshift_assign <- ">>>=")
    // 3 char operators
    | (op_lshift_assign <- "<<=")
    | (op_rshift_assign <- ">>=")
    | (op_signed_rshift <- ">>>")
    // 2 char operators
    | (op_lshift <- "<<")
    // ">>" will not be recognized, becase this parser can not distinguish ">>" from ">" ">"
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
    // single char operators
    | (op_star <- "*") // may mean mul or wildcard
    | (op_div <- "/")
    | (op_mod <- "%%")
    | (op_plus <- "+") // may mean add or sign plus
    | (op_minus <- "-") // may mean sub or sign minus
    | (op_amp <- "&amp;") // may mean bitwise
    | (op_logical_neg <- "!")
    | (op_complement <- "~")
    | (op_or <- "|")
    | (op_xor <- "^")
    | (op_assign <- "=")
    | (OL <- "<") // may mean less than or template parameter
    | (OG <- ">") // may mean greater than or template parameter
    | (ques <- "?") | (colon <- ":") | (dot <- ".");

TEXT scan= null <- multiline_comment | singleline_comment | " " | "&t;" | "&f;" | "&v;"| eol;

TEXT scan= (r_int <- r_long | r_short) | (r_double <- r_float) | (l_bool <- r_true | r_false)
    | (l_string <- word dot (word match "getString") LP l_string RP); // support for externalized string

TEXT scan= xcep(LB | RB | LP | RP | LK | RK) any 
    | (block <- LB *^ RB)
    | (param <- LP *^ RP)
    | (index <- LK *^ RK);

TEXT scan= word *(dot word) (template_param <- 
        OL
        ?(ques ((word match "super") | r_extends)) ^
        *((comma | op_amp) ?(ques ((word match "super") | r_extends)) ^)
        OG
    ) 
    | (null <- 
        OL 
        (word *(dot word) ((word match "super") | r_extends) ^ | ^) 
        *((comma | op_amp) (word *(dot word) ((word match "super") | r_extends) ^ | ^)) 
        OG
    )
    | word *(dot word) *index | ques *index
    | (block scan ^) | (param scan ^); // recurse into block, and param

TEXT scan= ?(null <- (word match "this") dot) (id <- word *(dot word xcep(param)) ?template_param)
    | (id <- (word match "this"))
    | (l_string <- l_string +(op_plus l_string))
    | (r_annotation_decl <- ("@" r_interface ))
    | (block scan ^) | (param scan ^) | (index scan ^); // recurse into block, index, and param

// remove package, import
TEXT scan= null <- r_package id semicolon | r_import id ?(dot op_star) semicolon;

TEXT scan=
    id (index match LK RK) op_assign (block scan ^) semicolon // T a[] = { ... };
    | r_new id *(dot id) +(index scan ^)
    | id +((index match LK RK) | (index scan ^))
    | id insert(dot) insert(m_get) (index match (LP <- LK) *(xcep(RK) ^) (RP <- RK))
    | dot (m_length <- m_size (param match LP RP)) 
    | dot m_length (null <- (param match LP RP))
    | (block scan ^) // recurse into block
    | (param scan ^) | (index scan ^); // recurse into expression

TEXT scan= (null <- r_private | r_public | r_protected | r_synchronized | r_final | r_abstract | r_strictfp | r_volatile | r_transient)
    | (null <- "@" id ?param)
    | (null <- r_static xcep(LB))
    | (null <- +(r_extends id *(comma id) | r_implements id *(comma id)))
    | (null <- r_throws id *(comma id))
    | (interface_block <- (def_block <- r_interface id ?(r_extends id *(comma id)) block))
    | (anotation_block <- (def_block <- r_annotation_decl id block))
    | (block scan ^) | (param scan ^); // recurse into block and param

// remove array initialization tables
TEXT scan= op_assign (initialization_block <- preq(block)) (null <- block) semicolon
    | index (initialization_block <- preq(block)) (null <- block)
    | (block scan ^) // recurse into block
    | (param scan ^) | (index scan ^); // recurse into expression

TEXT scan= xcep(id | param | index | l_float | l_int | block) any (null <- op_minus) // remove unary minus
    | (method_like <- m_charAt | m_compareTo | m_dispose | m_equals | m_getClass | m_get | m_hashCode | m_hasNext | m_iterator | m_length | m_next | m_run | m_size | m_toArray | m_toString)
    | ques insert(c_cond) // insert tokens for control-flow complexity counter
    | (block scan ^) // recurse into block
    | (param scan ^) | (index scan ^); // recurse into expression

// remove simple delegations; remove empty method definition; remove getter, setter; remove redundant paren of return statement; remove assertion
TEXT scan=
    (null <- (r_void | r_boolean | r_byte | r_char | r_double | r_float | r_int | r_short | r_object | r_string | id) *index
         (id | method_like) param ((block match LB ?r_return id dot id param semicolon RB) | (block match LB RB)))
    | (null <- (r_boolean | r_byte | r_char | r_double | r_float | r_int | r_short | r_object | r_string | id) *index
         (id | method_like) (param match LP RP) (block match LB r_return id semicolon RB))
    | (null <- r_void (id | method_like) param (block match LB id op_assign id semicolon RB))
    | r_return (param match (null <- LP) *(xcep(RP) any) (null <- RP)) semicolon
    | (null <- r_assert *(xcep(semicolon | eof) any) semicolon)
    | (block scan ^) | (param scan ^); // recurse into block and param

TEXT scan= 
    r_if param ((block scan ^) | (block <- insert(LB) ^ insert(RB))) *(r_else r_if param (block | (block <- insert(LB) ^ insert(RB)))) ?(r_else (block | (block <- insert(LB) ^ insert(RB)))) 
    | r_else (block | (block <- insert(LB) ^ insert(RB)))
    | r_while param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_for param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_do ((block scan ^) | (block <- insert(LB) ^ insert(RB))) r_while param semicolon
    | r_try (block scan ^) *((r_catch param | r_finally) (block scan ^))
    | (r_catch param | r_finally) (block scan ^)
    %(switch_statement_rule)s
    | *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_do | r_try | r_catch | r_finally | r_switch) any) semicolon
    | (block scan ^) | (param scan ^); // recurse into block and param

TEXT scan= 
    r_if param block *(r_else r_if param block) ?(r_else block)
    | r_else block
    | r_while param block
    | r_for param block
    | r_do block r_while param semicolon
    | r_switch param block
    | r_try block *((r_catch param | r_finally) block)
    | (r_catch param | r_finally) block
    | (simple_statement <- *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_do | r_try | r_catch | r_finally| r_switch | r_case | r_default) any) semicolon)
    | (block scan ^) | (param scan ^); // recurse into block and param

%(simple_statement_removal_rule)s

// enclose class/method/constructor definition by block
TEXT scan= (def_block <- r_class id (block scan ^))
    | (def_block <- r_new id param (block scan ^)) 
    | (def_block <- (r_void | r_int | r_long | r_short | r_double | r_float | r_boolean | r_char | r_byte | id) *(index match LK RK) (id | method_like) param (block scan ^))
    | (def_block <- id param (block scan ^)) // constructor
    | (block scan ^) | (param scan ^); // recurse into block and param

// insert tokens for control-flow complexity counter
TEXT scan= (r_if | r_switch) insert(c_cond) | (r_for | r_while) insert(c_loop)
    | (id | method_like) insert(c_func) (param scan ^)
    | (def_block scan ^) | (block scan ^) | (param scan ^) | (index scan ^) | (simple_statement scan ^);
""" % (locals())
        self.pat = easytorq.Pattern(patternStr)
        
        fmt = easytorq.CngFormatter()
        
        # parameter by default
        fmt.addreplace('id', 'id|%s')
        
        fmt.addreplace('id', 'id|%s')
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
        fmt.addflatten('method_like')
        if not optionValues['d']:
            # requires exact match
            fmt.addreplace('l_int', 'l_int=%s')
            fmt.addreplace('l_float', 'l_float=%s')
            fmt.addreplace('l_bool', 'l_bool=%s')
        else:
            # non parameter by default
            fmt.addreplace('l_bool', 'l_bool|%s')
            fmt.addreplace('l_int', 'l_int|%s')
            fmt.addreplace('l_float', 'l_float|%s')
            
        if optionValues['r']:
            fmt.addnone('interface_block')
        else:
            fmt.addflatten('interface_block')
        fmt.addnone('anotation_block')
        if not optionValues['s']:
            # requires exact match
            fmt.addreplace('l_string', 'l_string=%s')
            fmt.addreplace('l_char', 'l_char=%s')
        else:
            # non parameter by default
            fmt.addreplace('l_string', 'l_string|%s')
            fmt.addreplace('l_char', 'l_char|%s')

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
    return JavaPreprocessor()

if __name__ == '__main__':
    cnv = easytorq.ICUConverter()
    cnv.setencoding("char")
    
    f = file(sys.argv[1], "rb")
    str = f.read()
    f.close()
    
    strUtf8 = cnv.decode(str)
    
    prep = getpreprocessor()
    prep.setoptions('default')
    s = prep.parse(strUtf8)
    sys.stdout.write(s)
    
