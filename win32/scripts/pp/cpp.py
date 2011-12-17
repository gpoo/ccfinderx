#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import pp.preprocessor as pp
import easytorq
import sys

def default_option_values():
    return { 'k' : False }

def getname():
    return "cpp"

def getversion():
    return (2, 0, 0, 2)

def recursivereplace(s, beforeafterpairs):
    if not beforeafterpairs:
        return s
    bef, aft = beforeafterpairs[0]
    return aft.join(recursivereplace(t, beforeafterpairs[1:]) for t in s.split(bef))

class CppPreprocessor(pp.Base):
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
            simple_statement_removal_rule = "(null <- simple_statement) |"
        
        patternStr = """TEXT scan= 
    preq("&(a-z);") (
        (op_logical_and <- "and")
        | (op_and_assign <- "and_eq")
        | (m_abort <- "abort")
        | (r_auto <- "auto")
        | (r_amp <- "bitand")
        | (m_assert <- "assert")
        | (r_or <- "bitor")
        | (r_bool <- "bool")
        | (r_break <- "break")
        | (r_case <- "case")
        | (r_catch <- "catch")
        | (r_char <- "char")
        | (r_class <- "class")
        | (op_complement <- "compl")
        | (r_const_cast <- "const_cast") | (r_const <- "const")
        | (r_continue <- "continue")
        | (r_default <- "default")
        | (r_delete <- "delete")
        | (r_dynamic_cast <- "dynamic_cast")
        | (r_double <- "double") | (r_do <- "do")
        | (r_else <- "else")
        | (r_enum <- "enum")
        | (m_exit <- "exit")
        | (r_explicit <- "explicit")
        | (r_extern <- "extern")
        | (r_false <- "false")
        | (r_float <- "float")
        | (r_for <- "for")
        | (r_friend <- "friend")
        | (r_goto <- "goto")
        | (r_if <- "if")
        | (r_inline <- "inline")
        | (r_intmax <- "intmax_t")
        | (r_intptr <- "intptr_t")
        | (r_int64 <- ("int64_t" | "int_least64_t" | "int_fast64_t"))
        | (r_int32 <- ("int32_t" | "int_least32_t" | "int_fast32_t"))
        | (r_int16 <- ("int16_t" | "int_least16_t" | "int_fast16_t"))
        | (r_int8 <- ("int8_t" | "int_least8_t" | "int_fast8_t"))
        | (r_int <- "int")
        | (m_longjmp <- "longjmp")
        | (r_long <- "long")
        | (r_mutable <- "mutable")
        | (r_namespace <- "namespace")
        | (r_new <- "new")
        | (op_logical_neg <- "not")
        | (op_ne <- "not_eq")
        | (m_offsetof <- "offsetof")
        | (r_operator <- "operator")
        | (op_logical_or <- "or")
        | (op_or_assign <- "or_eq")
        | (r_private <- "private")
        | (r_protected <- "protected")
        | (m_ptrdiff_t <- "ptrdiff_t")
        | (r_public <- "public")
        | (r_register <- "register")
        | (r_reinterpret_cast <- "reinterpret_cast")
        | (r_restrict <- "restrict")
        | (r_return <- "return")
        | (r_short <- "short")
        | (m_setjmp <- "setjmp")
        | (r_signed <- "signed")
        | (r_sizeof <- "sizeof")
        | (m_size_t <- "size_t")
        | (r_static <- "static")
        | (r_static_cast <- "static_cast")
        | (r_struct <- "struct")
        | (r_switch <- "switch")
        | (r_template <- "template")
        // | (r_this <- "this") // keyword "this" is treated as an identifier
        | (r_throw <- "throw")
        | (r_true <- "true")
        | (r_try <- "try")
        | (r_typedef <- "typedef")
        | (r_typeid <- "typeid")
        | (r_typename <- "typename")
        | (r_union <- "union")
        | (r_unsigned <- "unsigned")
        | (r_uintmax <- "uintmax_t")
        | (r_uintptr <- "uintptr_t")
        | (r_uint64 <- ("uint64_t" | "uint_least64_t" | "uint_fast64_t"))
        | (r_uint32 <- ("uint32_t" | "uint_least32_t" | "uint_fast32_t"))
        | (r_uint16 <- ("uint16_t" | "uint_least16_t" | "uint_fast16_t"))
        | (r_uint8 <- ("uint8_t" | "uint_least8_t" | "uint_fast8_t"))
        | (r_using <- "using")
        | (r_virtual <- "virtual")
        | (r_void <- "void")
        | (r_volatile <- "volatile")
        | (m_wchar_t <- "wchar_t")
        | (r_while <- "while")
        | (op_xor <- "xor")
        | (op_xor_assign <- "xor_eq")
        | (m_assert <- "assert")
    ) xcep("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);")
    | (word <- ("&(a-z);" | "&(A-Z);" | "_") *("&(a-z);" | "&(A-Z);" | "_" | "&(0-9);"))
    | (multiline_comment <- "/*" *(xcep("*/") any) "*/")
    | (singleline_comment <- "//" *(xcep(eol) any) preq(eol))
    | (l_string <- ?"L" "&quot;" *("&bslash;" any | xcep("&quot;" | eol) any) "&quot;")
    | (l_char <- ?"L" "&squot;" *("&bslash;" any | xcep("&squot;" | eol) any) "&squot;")
    | (l_float <- (
            +"&(0-9);" "." *"&(0-9);" ?(("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "l" | "F" | "L") 
            | +"&(0-9);" ("e" | "E") ?("-" | "+") +"&(0-9);") ?("f" | "l" | "F" | "L") 
            | +"&(0-9);" ("f" | "F") ? ("l" | "L")
    )
    | (l_int <- (("0x" | "0X") +("&(0-9);" | "&(a-f);" | "&(A-F);") | +"&(0-9);") *("u" | "l" | "U" | "L"))
    | (macro_line <- "#" *("&bslash;" *(" " | "&t;") eol | xcep(eol | eof | "/*" | "//") any | (multiline_comment <- "/*" *(xcep(eof | "*/") any) "*/")) preq(eol | eof | "//"))
    | (semicolon <- ";")
    | (comma <- ",") 
    | (LB <- "{") | (RB <- "}") 
    | (LP <- "(") | (RP <- ")") 
    | (LK <- "[") | (RK <- "]") 
    // 3 char operators
    | (op_lshift_assign <- "<<=")
    | (op_rshift_assign <- ">>=")
    | (op_pointer_to_member_from_pointer <- "->*")
    // 2 char operators
    | (op_scope_resolution <- "::")
    | (op_lshift <- "<<")
    | (op_rshift <- ">>")
    | (op_increment <- "++")
    | (op_decrement <- "--")
    | (op_member_access_from_pointer <- "->")
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
    | (op_poiner_to_member_from_reference <- ".*")
    | (op_logical_and <- "&amp;" "&amp;")
    | (op_logical_or <- "||")
    // single char operators
    | (op_star <- "*") // may mean mul or indirection
    | (op_div <- "/")
    | (op_mod <- "%%")
    | (op_plus <- "+") // may mean add or sign plus
    | (op_minus <- "-") // may mean sub or sign minus
    | (op_amp <- "&amp;") // may mean bitwise and or indirection
    | (op_logical_neg <- "!")
    | (op_complement <- "~")
    | (op_or <- "|")
    | (op_xor <- "^")
    | (op_assign <- "=")
    | (OL <- "<") // may mean less than or template parameter
    | (OG <- ">") // may mean greater than or template parameter
    | (ques <- "?") | (colon <- ":") | (dot <- ".");

TEXT scan= (null <- macro_line | multiline_comment | singleline_comment | " " | "&t;" | "&f;" | "&bslash;" *(" " | "&t;") eol | eol)
    | (r_int <- (r_intmax | r_intptr | r_int64 | r_int32 | r_int16))
    | (r_int <- (r_uintmax | r_uintptr | r_uint64 | r_uint32 | r_uint16))
    | (r_int <- m_wchar_t)
    | (r_char <- r_int8)
    | (r_char <- r_uint8);

TEXT scan= preq(r_operator) 
    (
        (word <- r_operator comma)
        | (word <- r_operator (op_logical_neg | op_logical_and | op_logical_or))
        | (word <- r_operator (op_ne | op_eq | OG | OL | op_ge | op_le))
        | (word <- r_operator op_mod)
        | (word <- r_operator (op_mod_assign | op_and_assign | op_add_assign | op_mul_assign | op_add_assign | op_sub_assign | op_div_assign | op_lshift_assign | op_assign | op_rshift_assign | op_xor_assign))
        | (word <- r_operator (op_amp | op_star))
        | (word <- r_operator LP RP)
        | (word <- r_operator (op_plus | op_minus))
        | (word <- r_operator (op_increment | op_decrement))
        | (word <- r_operator (op_member_access_from_pointer | op_pointer_to_member_from_pointer))
        | (word <- r_operator op_div)
        | (word <- r_operator (op_lshift | op_rshift))
        | (word <- r_operator LK RK)
        | (word <- r_operator op_xor)
        | (word <- r_operator op_complement)
        | (word <- r_operator (r_delete | r_new))
        | (word <- r_operator r_bool)
    );

TEXT scan=
    (r_int <- (r_signed | r_unsigned)(r_long r_long r_int | r_long r_int | r_short r_int | r_int))
    | (r_int <- (r_signed | r_unsigned)(r_long r_long | r_long | r_short))
    | (r_char <- (r_signed | r_unsigned) r_char)
    | (r_int <- r_signed | r_unsigned)
    | (r_int <- r_long r_long | r_long | r_short)
    | (r_int <- m_size_t | m_ptrdiff_t | wchar_t)
    | (r_float <- r_long r_double | r_double)
    | (l_int <- (word match "NULL"))
    | (l_bool <- r_true | r_false)
    | (l_string <- +l_string)
    | (null <- (r_private | r_public | r_protected) colon)
    | (null <- r_virtual | r_inline | r_static)
    | (word <- op_scope_resolution word *(op_scope_resolution word) ?(op_scope_resolution op_complement word))
    | (word <- word +(op_scope_resolution word) ?(op_scope_resolution op_complement word))
    | (word <- word op_scope_resolution op_complement word);

TEXT scan= xcep(LB | RB | LP | RP | LK | RK) any
    | (block <- LB *^ RB) 
    | (null <- LP op_star) *^ (null <- RP) (op_member_access_from_pointer <- dot) 
    | (index <- LK *^ RK)
    | (param <- (LP (null <- r_void) RP | LP *^ RP));

TEXT scan= xcep(OL | OG | block | param | semicolon) any | (template_param <- OL *^ OG) 
    | (block scan ^) | (param scan ^) | (index scan ^); // recurse into block and param

TEXT scan= ?(null <- (word match "this" op_member_access_from_pointer))
        (id <- word ?(null <- template_param) *((dot | op_member_access_from_pointer) word xcep(param)) ?(null <- template_param))
    | (id <- (word match "this"))
    | (r_const_cast | r_dynamic_cast | r_reinterpret_cast | r_static_cast) (null <- template_param)
    | (block scan ^) | (param scan ^) | (index scan ^); // recurse into block and param

TEXT scan= op_assign (initialization_block <- preq(block)) (null <- block) semicolon
    | (r_class | r_struct) id (null <- colon *(r_public | r_private | r_protected | r_virtual) id *(comma *(r_public | r_private | r_protected | r_virtual) id))
    | (null <- r_enum ?id block)
    | (null <- m_assert param semicolon)
    | r_return (param match (null <- LP) *(xcep(RP) any) (null <- RP)) semicolon
    | (block scan ^); // recurse into block

TEXT scan= xcep(id | param | RK | l_float | l_int | block) any (null <- op_minus)
    | (null <- r_struct | r_union | r_enum) id xcep(block | colon)
    | ques insert(c_cond) // insert tokens for control-flow complexity counter
    | (block scan ^) // recurse into block
    | (param scan ^) | (index scan ^); // recurse into expression

TEXT scan= (value_list <- (l_bool | l_string | l_int | l_char | l_float | id) +(comma (l_string | l_int | l_char | l_float | id) ?comma))
    | (block scan ^);

TEXT scan= 
    r_if param ((block scan ^) | (block <- insert(LB) ^ insert(RB))) *(r_else r_if param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))) ?(r_else ((block scan ^) | (block <- insert(LB) ^ insert(RB)))) 
    | r_else ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_while param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_for param ((block scan ^) | (block <- insert(LB) ^ insert(RB)))
    | r_do (block | (block <- insert(LB) ^ insert(RB))) r_while param semicolon
    | r_try (block scan ^) *(r_catch param (block scan ^))
    | r_catch (block scan ^)
    %(switch_statement_rule)s
    | (r_return | r_break | r_continue | op_assign) *(xcep(block | LB | semicolon) any) semicolon
    | (null <- (r_friend | r_typedef) *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_do | r_try | r_catch | r_switch) any) semicolon)
    | (null <- r_using r_namespace id semicolon)
    | (null <- r_namespace op_eq id semicolon)
    | *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_do | r_try | r_catch | r_switch) any) semicolon
    | (block scan ^); // recurse into block
    
TEXT scan= 
    r_if param block *(r_else r_if param block) ?(r_else block) 
    | r_else block
    | r_while param block
    | r_for param block
    | r_do block r_while param semicolon
    | r_try block *(r_catch param block)
    | r_catch block
    | r_switch param block
    | (simple_statement <- *(xcep(block | LB | semicolon | r_if | r_while | r_for | r_do | r_try | r_catch | r_switch | r_case | r_default) any) semicolon)
    | (block scan ^); // recurse into block
    
TEXT scan= (simple_statement match (r_return | r_continue | r_break | r_throw) +any)
    | %(simple_statement_removal_rule)s
    // mark simple getter/setter/delegation/empty block
    (+(r_void | r_int | r_char | r_float | r_bool | r_class | r_struct | r_enum | r_union | r_const | r_volatile | op_star | op_amp | index | id) 
        param ?r_const ?(r_throw param)
        ?(colon id param *(comma id param)))
        (getter_body <- (block match LB (simple_statement match r_return ?(id op_member_access_from_pointer) id ?param semicolon) RB)
            | (block match LB (simple_statement match ?(id op_member_access_from_pointer) id param semicolon) RB)
            | ( block match LB RB)) 
    | r_namespace id (block scan ^)
    | r_extern l_string (block scan ^) // recurse into extern "C" block
    | (r_struct | r_union) ?id (block scan ^)
    | (r_class | r_struct | r_union) id (block scan ^); // recurse into top level of class definition

// enclose class/method/function definition by block
TEXT scan= (
        ?(null <- +(r_template template_param)) (
            (null <- (r_class | r_struct | r_union) ?id (block match LB RB)) // remove empty structure definition
            | (def_block <- r_class id (block scan ^))
            | (def_block <- (r_struct | r_union) ?id (block scan ^))
            | (null <- +(r_void | r_int | r_char | r_float | r_bool | r_class | r_struct | r_enum | r_union | r_const | r_volatile | op_star | op_amp | index | (id <- op_complement id) | ?r_typename id) insert(c_func) param ?r_const ?(null <- (r_throw param))
                ?(colon id param *(comma id param))
                getter_body)
            | (def_block <- +(r_int | r_char | r_float | r_bool | r_class | r_struct | r_enum | r_union | r_const | r_volatile | op_star | op_amp | index | (id <- op_complement id) | ?r_typename id ) insert(c_func) param ?r_const ?(null <- (r_throw param))
                ?(null <- colon id param *(comma id param)) // remove initialization list of constructor
                (block scan ^))
            | (def_block <- r_void id insert(c_func) param ?r_const ?(null <- (r_throw param)) (block scan ^))
        )
    )
    | (block scan ^);

// insert tokens for control-flow complexity counter
TEXT scan= (r_if | r_switch) insert(c_cond) | (r_for | r_while) insert(c_loop)
    | (def_block scan ^) | (block scan ^); // recurse into block
TEXT scan= (id | r_int | r_char | r_float | r_bool) id (param scan ^) *(comma id ?(param scan ^)) 
    | id insert(c_func) (param scan ^)
    | (def_block scan ^) | (block scan ^) | (simple_statement scan ^) | (param scan ^) | (index scan ^); // recurse into block, simple_statement, param, index
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
        fmt.addterminate('macro_line')
        fmt.addflatten('simple_statement')
        fmt.addreplace('semicolon', 'suffix:semicolon')
        fmt.addreplace('colon', 'suffix:colon')
        fmt.addformat('def_block', '(def_block', ')def_block')
        fmt.addflatten('value_list')
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
        IDCOLONCOLON = "id|::"
        len_IDCOLONCOLON = len(IDCOLONCOLON)
        
        if self.pat == None:
            self.setoptions(None)
        
        t = easytorq.Tree(sourceCodeStrInUtf8)
        
        self.pat.apply(t)
        lines = self.fmt.format(t).split('\n')
        for li, line in enumerate(lines):
            if line:
                fields = line.split('\t')
                if fields[2].startswith("id|"):
                    f2 = fields[2]
                    if f2.startswith(IDCOLONCOLON):
                        f2 = "id|" + f2[len_IDCOLONCOLON:]
                    fields[2] = f2
                    #fields[2] = recursivereplace(f2, [ ("operator-&gt;", "operator-&gt;"), ("-&gt;", ".") ])
                    lines[li] = '\t'.join(fields)
        return '\n'.join(lines)
    

def getpreprocessor():
    return CppPreprocessor()

if __name__ == '__main__':
    cnv = easytorq.ICUConverter()
    cnv.setencoding("char")
    
    f = file(sys.argv[1], "rb")
    str = f.read()
    f.close()
    
    strUtf8 = cnv.decode(str)
    
    prep = getpreprocessor()
    prep.setoptions("")
    s = prep.parse(strUtf8)
    sys.stdout.write(s)
    
