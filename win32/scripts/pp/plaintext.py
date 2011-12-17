#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import pp.preprocessor as pp
import easytorq
import sys

def getname():
    return "plaintext"

def getversion():
    return (2, 0, 0, 0)

class PlaintextPreprocessor(pp.Base):
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
        
        patternStr = """TEXT scan= (
    chars <- +("&(a-z);" | "&(A-Z);" | "&(0-9);")
) | (
    space <- +(
        "&#x(0-20);" 
        | "&#x7f" 
        | "&#x(80-a0);"
        | "&#x(2000-200f);"
        | "&#x3000"
        | eol
    )
) | (
    punct <-
        "&#x(21-2f);"
        | "&#x(3a-3f);"
        | "&#x(5b-5f);"
        | "&#x(7b-7e);"
        | "&#x(a1-bf);"
        | "&#x(2010-205f);"
        | "&#x(20a0-20b5);"
        | "&#x(2190-21ff);"
        | "&#x(2200-22ff);"
        | "&#x(2300-23db);"
        | "&#x(2400-2426);"
        | "&#x(2440-244a);"
        | "&#x(2600-26b1);"
        | "&#x(2701-27be);"
        | "&#x(2a00-2aff);"
        | "&#x(27c0-27ef);"
        | "&#x(27f0-27ff);"
        | "&#x(2900-297f);"
        | "&#x(2980-29ff);"
        | "&#x(2b00-2b13);"
        | "&#x(2500-257f);"
        | "&#x(2580-259f);"
        | "&#x(25a0-25ff);"
        | "&#x(2e00-2e17);"
        | "&#x(3001-303f);"
        | "&#x(4dc0-4dff);"
        | "&#x(fe10-fe19);"
        | "&#x(ff01-ff0f);"
        | "&#x(ff01-ff0f);"
        | "&#x(ff1a-ff1f);"
        | "&#x(ff3b-ff3f);"
        | "&#x(ff5b-ff65);"
        | "&#x(ffe0-ffee);"
        | "&#x(1d300-1d356);"
) | (
        chars <- xcep(eof) any
);

TEXT scan= (null <- space) | (word <- +chars);
"""
        self.pat = easytorq.Pattern(patternStr)
        
        fmt = easytorq.CngFormatter()
        fmt.addreplace('punct', 't/%s')
        fmt.addreplace('word', 't/%s')
        self.fmt = fmt
    
    def getnormalizedoptionstring(self):
        return 'default'
    
    def getdefaultparameterizing(self):
        return dict()
    
    def parse(self, sourceCodeStrInUtf8):
        if self.pat == None:
            self.setoptions(None)
        
        t = easytorq.Tree(sourceCodeStrInUtf8)
        self.pat.apply(t)
        s = self.fmt.format(t)
        return s
    
def getpreprocessor():
    return PlaintextPreprocessor()

if __name__ == '__main__':
    cnv = easytorq.ICUConverter()
    cnv.setencoding("char")
    
    f = file(sys.argv[1], "rb")
    str = f.read()
    f.close()
    
    strUtf8 = cnv.decode(str)
    
    prep = preprocessor()
    prep.setoptions('default')
    s = prep.parse(strUtf8)
    sys.stdout.write(s)
    
