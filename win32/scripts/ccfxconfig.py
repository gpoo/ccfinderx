#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import sys
import os
import os.path
import string

import utility

class Infos:
    def __init__(self):
        if sys.platform in ( 'win32' ):
            self.ccfxFname = 'ccfx.exe'
        elif sys.platform in ( 'linux2' ):
            self.ccfxFname = 'ccfx'
        else:
            raise IOError, 'Platform not supported'
        
        fileAbsPath = os.path.abspath(__file__)
        moduleDir = os.sep.join(fileAbsPath.split(os.sep)[:-1])
        fs = string.split(moduleDir, os.sep)
        self.ccfxDir = string.join(fs[:-1], os.sep)
        
        self.ccfxPath = string.join([ self.ccfxDir, self.ccfxFname ], os.sep)
        
        args = utility.escapeCommandline([ self.ccfxPath, "f", "-p" ])
        #print "args=", args
        p = os.popen(string.join(args, " "))
        preps = p.read().split('\n')
        if p.close() != None:
            raise IOError, "fail to invoke ccfx"
        if preps[-1] == '':
            preps = preps[:-1]
        self.preprocessScripts = preps

if __name__ == '__main__':
    infos = Infos()
    print "ccfx directory = %s" % infos.ccfxDir
    print "ccfx fname = %s" % infos.ccfxFname
    print "ccfx preprpcess scripts = %s" % infos.preprocessScripts

