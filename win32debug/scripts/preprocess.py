#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Copyright: This module has been placed in the public domain.

import sys
import os
import os.path
import stat
import imp
import getopt
import textwrap
import time
import threading
import subprocess
import tempfile
import platform
import json

import pp.preprocessor as pp
import easytorq
import utility
import threadingutil
import moduleloadutility

# globals
__mlu = moduleloadutility.ModuleLoadUtility()

def uniq(seq):
    r = list()
    if len(seq) == 0:
        return r
    
    r.append(seq[0])
    lastItem = seq[0]
    for item in seq[1:]:
        if item != lastItem:
            r.append(item)
        lastItem = item
    return r

CCFX_PREPDIR = ".ccfxprepdir"

# walkaround to avoid \x5c character problem in os.path.split
def os_path_split(pathstr, cnv):
    s = cnv.decode(pathstr)
    d, f = os.path.split(s)
    return cnv.encode(d), cnv.encode(f)

# walkaround to avoid limit of length of file path in Windows
if platform.architecture() == ('32bit', 'WindowsPE'):
    import win32getshortname
    
    __converter_file_funcs = easytorq.ICUConverter()
    __converter_file_funcs.setencoding("char")
    
    def __shorten(filepath):
        dirName, fileName = os_path_split(filepath, __converter_file_funcs)
        shortDirName = win32getshortname.getShortPathName(dirName) # may return None
        if shortDirName:
            fp = os.path.join(shortDirName, fileName)
            if fp:
                return fp
        return filepath
    
    def fopen(filepath, modestr): return file(__shorten(filepath), modestr)
    def remove_file(filepath): os.remove(__shorten(filepath))
    def rename_file(src, dst): os.rename(__shorten(src), __shorten(dst))
    def stat_file(filepath): return os.stat(__shorten(filepath))
else:
    fopen = file
    remove_file = os.remove
    rename_file = os.rename
    stat_file = os.stat

def remove_file_neglecting_error(filepath):
    try:
        remove_file(filepath)
    except:
        pass

def make_temp_filename(output, cnv):
    suffix = ".tmp"
    value = os.environ.get("CCFINDER_TEMPORARY_DIRECTORY")
    if value != None:
        return tempfile.mktemp(suffix=suffix, dir=value)
    elif output:
        d = os_path_split(output, cnv)[0]
        return tempfile.mktemp(suffix=suffix, dir=d)
    else:
        return tempfile.mktemp(suffix=suffix)

def calc_extension(preprocessorName, preprocessModule, preprocessorOptions):
    prep = preprocessModule.getpreprocessor()
    prepVersion = preprocessModule.getversion()
    
    if preprocessorOptions:
        prep.setoptions(preprocessorOptions)
    optionStr = prep.getnormalizedoptionstring()
    
    checkStr = prep.tonormalizedoptionstring(optionStr)
    assert checkStr == optionStr
    
    return ".%s.%s.%s.ccfxprep" % (preprocessorName, pp.to_version_str(prepVersion), optionStr)

def build_default_option_string(jsonData):
    defaultOptionStr = ""
    for name, value in sorted(jsonData.iteritems()):
        widgetType = value[0]
        if widgetType == "checkbox":
            isCheckedByDefault, valueChecked, valueUnchecked, description = value[1:]
            if isCheckedByDefault:
                defaultOptionStr += valueChecked
            else:
                defaultOptionStr += valueUnchecked
        else:
            assert False
    return defaultOptionStr

def build_commandline_help_string(jsonData):
    def format_paragraph(s, max_chars = 78, first_line_indent = 0, line_indent = 4):
        return textwrap.wrap(s, max_chars, 
                initial_indent = " " * first_line_indent, 
                subsequent_indent = " " * line_indent)

    r = list()
    r.append("Options (default -r %s)" % build_default_option_string(jsonData))
    for name, value in sorted(jsonData.iteritems()):
        widgetType = value[0]
        if widgetType == "checkbox":
            isCheckedByDefault, valueChecked, valueUnchecked, description = value[1:]
            r.append("\n".join(format_paragraph("%s+: %s" % (name, description))))
        else:
            assert False
    
    return "\n".join(r)

def print_preprocess_script_option_description(preprocessorName, argv):
    options, args = getopt.gnu_getopt(argv, "so:")
    optionRawString = None
    outputFile = None
    for name, value in options:
        if name == '-s':
            optionRawString = True
        elif name == '-o':
            outputFile = value
    
    m = __mlu.load("pp." + preprocessorName)
    prep = m.getpreprocessor()
    
    prepVersion = m.getversion()
    versionStr = ".".join([ str(i) for i in prepVersion])
    
    if not optionRawString:
        try:
            t = json.loads(prep.getoptiondescription())
        except ValueError:
            descriptionStr = prep.getoptiondescription()
        else:
            descriptionStr = build_commandline_help_string(t)
    else:
        descriptionStr = prep.getoptiondescription()
    
    s = "%s %s\n%s" % (preprocessorName, versionStr, descriptionStr)
    if outputFile:
        f = fopen(outputFile, "w")
        print >> f, s
        f.flush()
        f.close()
    else:
        print s

def print_postfix_output(preprocessorName, argv):
    options, args = getopt.gnu_getopt(argv, "r:o:")
    preprocessorOptions = None
    outputFile = None
    for name, value in options:
        if name == '-r':
            preprocessorOptions = value
        elif name == '-o':
            outputFile = value
    if len(args) > 0:
        print >> sys.stderr, "error: too many command-line arguments (1)"
        sys.exit(1)
    
    preprocessModule = __mlu.load("pp." + preprocessorName)
    extensionStr = calc_extension(preprocessorName, preprocessModule, preprocessorOptions)
    
    if outputFile:
        f = fopen(outputFile, "w")
        print >> f, extensionStr
        f.flush()
        f.close()
    else:
        print extensionStr

def print_default_parameterizing(preprocessorName, argv):
    options, args = getopt.gnu_getopt(argv, "o:")
    outputFile = None
    for name, value in options:
        if name == '-o':
            outputFile = value
    if len(args) > 0:
        print >> sys.stderr, "error: too many command-line arguments (2)"
        sys.exit(1)
    
    preprocessModule = __mlu.load("pp." + preprocessorName)
    prep = preprocessModule.getpreprocessor()
    parameterizingTable = prep.getdefaultparameterizing()

    if outputFile:
        f = fopen(outputFile, "w")
    else:
        f = sys.stdout
    
    for k, v in sorted(parameterizingTable.iteritems()):
        if v == pp.Param.ANY_MATCH:
            s = "ANY_MATCH"
        elif v == pp.Param.EXACT_MATCH:
            s = "EXACT_MATCH"
        elif v == pp.Param.P_MATCH:
            s = "P_MATCH"
        else:
            print >> sys.stderr, "error: internal. invalid parameterizing table"
            sys.exit(1)
        print >> f, "%s\t%s" % ( k, s )
    
    if outputFile:
        f.flush()
        f.close()

def to_filename_in_prepdir(preprocessedFname, prepDirs):
    for pd in prepDirs:
        len_pd = len(pd)
        if preprocessedFname.startswith(pd) and preprocessedFname[len_pd] == os.sep:
            return pd + os.sep + CCFX_PREPDIR + preprocessedFname[len_pd:]
    return preprocessedFname

def find_preprocessed_files_iter(fileNames, extensionStr, prepDirs, find_not_exist, find_obsolute):
    for i, fname in enumerate(fileNames):
        preprocessedFname = fname + extensionStr
        preprocessedFname = to_filename_in_prepdir(preprocessedFname, prepDirs)
        
        try:
            sourceFileModifiedTime = stat_file(fname).st_mtime
        except:
            print >> sys.stderr, "error: fail to access file: %s" % fname
            sys.exit(1)
        try:
            prepFileModifiedTime = stat_file(preprocessedFname).st_mtime
        except:
            if find_not_exist:
                yield fname, preprocessedFname
        else:
            if find_obsolute and prepFileModifiedTime <= sourceFileModifiedTime:
                yield fname, preprocessedFname

def invoke_subprocess(*args):
    subp = subprocess.Popen(args, shell=True, stdin=None, stdout=None, stderr=None)
    subp.wait()
    return subp.returncode

class FileReader(threading.Thread):
    __slots__ = [ 'filename', 'error', 'content' ]
    
    def __init__(self, filename):
        threading.Thread.__init__(self)
        self.filename = filename
        self.error = None
        self.content = None
    
    def run(self):
        try:
            f = fopen(self.filename, "rb")
        except IOError, e:
            self.error = e
        else:
            self.content = f.read()
            f.close()

class __theMain(object):
    def __init__(self, mlu):
        syscnv = easytorq.ICUConverter()
        syscnv.setencoding("char")
        self.__syscnv = syscnv
        
        self.__mlu = mlu
        
        d, f = os_path_split(sys.argv[0], self.__syscnv)
        usage = """
Usage 1: %(argv0)s PREPROCESS_SCRIPT OPTIONS -i filelist
  Performs preprocessing.
Options
  -c encodingName: character encoding in source file.
  -n dir: directory where preprocessed files are created.
  -r value: preprocess options.
  -v: verbose.
  --removeobsolute: removes obsolete preprocessed files.
  --threads=maxWorkerThreads: uses up to maxWorkerThreads processes.
  --errorfiles=output: don't stop preprocessing when parse error occured, and
      writes such file names to output (redirected to stderr if '-' is given).
Usage 2: %(argv0)s PREPROCESS_SCRIPT --listoptions OPTIONS
  Prints out available options of the preprocess script.
Options
  -s: prints raw (unformatted) text.
Usage 3: %(argv0)s PREPROCESS_SCRIPT --preprocessedextension OPTIONS
  Prints out extension of preprocessed file.
Options
  -o output: output file.
  -r value: preprocess options.
Usage 4: %(argv0)s PREPROCESS_SCRIPT --getdefaultparameterizing OPTIONS
  Prints out default matching option for each parameter names.
Options
  -o output: output file.
""" % { "argv0" : f }
        usage = usage[1:-1]
        self.__usage = usage
        
    def main(self, argv):
        if len(argv) <= 1 or argv[1] == '-h':
            print self.__usage
            sys.exit(0)
        
        args = argv[1:]
        
        preprocessorName, args = args[0], args[1:]
        if not args:
            print >> sys.stderr, "error: invalid command-line arguments"
            sys.exit(1)
        if args[0] == '--listoptions':
            print_preprocess_script_option_description(preprocessorName, args[1:])
            sys.exit(0)
        elif args[0] == '--preprocessedextension':
            print_postfix_output(preprocessorName, args[1:])
            sys.exit(0)
        elif args[0] == '--getdefaultparameterizing':
            print_default_parameterizing(preprocessorName, args[1:])
            sys.exit(0)
        
        for i, a in enumerate(args):
            if a == "-n-":
                del args[i]
                print >> sys.stderr, "warning: option -n- is deprecated. neglect it"
                break # for
        
        options, args = getopt.gnu_getopt(args, "r:c:i:vn:", 
                [ "threads=", "removeobsolete", "errorfiles=" ])
        for arg in args:
            print >> sys.stderr, "error: too many command-line arguments (2)"
            sys.exit(1)
        
        preprocessorOptions = None
        encodingName = None
        filelistPath = None
        verbose = False
        maxWorkerThreads = 0
        prepDirs = list()
        optionRemoveObsolete = None
        optionParseErrorOutput = None
        for name, value in options:
            if name == '-r':
                preprocessorOptions = value
            elif name == '-c':
                encodingName = value
            elif name == '-i':
                filelistPath = value
            elif name == '-v':
                verbose = True
            elif name == '-w' or name == "--threads":
                maxWorkerThreads = int(value)
            elif name == "-n":
                prepDirs.append(value)
            elif name == "--removeobsolete":
                optionRemoveObsolete = True
            elif name == "--errorfiles":
                optionParseErrorOutput = value
            else:
                assert False
        if not filelistPath:
            print >> sys.stderr, "error: no file list is given"
            sys.exit(1)
        fileNames = list()
        for f in fopen(filelistPath, "r").readlines():
            f = f.strip()
            if f.startswith("-n "):
                dname = f[len("-n "):].strip()
                prepDirs.append(dname)
            else:
                fileNames.append(f)
    
        preprocessModule = self.__mlu.load("pp." + preprocessorName)
        extensionStr = calc_extension(preprocessorName, preprocessModule, preprocessorOptions)
        
        if verbose:
            verboseOutput = sys.stderr
        else:
            verboseOutput = None
        
        for d in (prepDirs or []):
            pd = d + os.sep + CCFX_PREPDIR
            try:
                os.makedirs(pd)
            except:
                pass # when the directory exists, error occurs.
            if not os.access(pd, os.F_OK | os.W_OK):
                print >> sys.stderr, "error: directory inaccessible '%s'" % d
                sys.exit(2)
        
        if optionRemoveObsolete:
            for fname, preprocessedFname in find_preprocessed_files_iter(
                    fileNames, extensionStr, prepDirs, False, True):
                remove_file(preprocessedFname)
            sys.exit(0)
        
        parseErrorFiles = None
        if optionParseErrorOutput:
            parseErrorFiles = []
        
        filesToBePreprocessed = [ fname for fname, preprocessedFname in find_preprocessed_files_iter(
                fileNames, extensionStr, prepDirs, True, True) ]
        filesToBePreprocessed = uniq(sorted(filesToBePreprocessed))
        
        if len(filesToBePreprocessed) > 0:
            options = list()
            if preprocessorOptions:
                options.append(( "-r", preprocessorOptions ))
            if encodingName:
                options.append(( "-c", encodingName ))
            for prepDir in prepDirs:
                options.append(( "-n", prepDir ))
            
            if maxWorkerThreads >= 2:
                self.__preprocess_files_by_workers(maxWorkerThreads, 
                        filesToBePreprocessed, extensionStr, preprocessModule, filelistPath,
                        options, verbose = verbose, parseErrorFiles = parseErrorFiles)
            else:
                self.__preprocess_files(filesToBePreprocessed, extensionStr, preprocessModule, filelistPath,
                        options, verbose = verbose, parseErrorFiles = parseErrorFiles)
        
        if optionParseErrorOutput:
            parseErrorFiles.sort()
            if parseErrorFiles:
                if optionParseErrorOutput == '-':
                    if parseErrorFiles:
                        print >> sys.stderr, "info: error files:"
                        for en in parseErrorFiles:
                            print >> sys.stderr, "  %s" % en
                else:
                    f = fopen(optionParseErrorOutput, "w")
                    if not f:
                        print >> sys.stderr, "error: can't create a file '%s'" % optionParseErrorOutput
                        sys.exit(2)
                    f.write("\n".join(parseErrorFiles))
                    f.close()
        
        sys.exit(0)
        
    def __preprocess_files_by_workers(self,
            maxWorkerThreads, filesToBePreprocessed, extensionStr, preprocessModule, 
            tempFileSeed = None, options = list(), verbose = False, 
            parseErrorFiles = None):
        assert maxWorkerThreads >= 2
        
        filesToBePreprocessed = list(filesToBePreprocessed)
        
        chunkSize = 200
        chunkSizeMax = 2000
        s2 = len(filesToBePreprocessed) / 64
        if s2 > chunkSize:
            chunkSize = s2
        if chunkSize > chunkSizeMax:
            chunkSize = chunkSizeMax
        
        commands = list()
        tempFiles = list()
        fi = 0
        while fi < len(filesToBePreprocessed):
            fiStart, fiEnd = fi, min(fi + chunkSize, len(filesToBePreprocessed))
            fi += chunkSize
            cmd = [ sys.executable, __file__, preprocessModule.getname() ]
            for k, v in options:
                cmd.append(k)
                cmd.append(v)
            fn = make_temp_filename(tempFileSeed, self.__syscnv)
            tempFiles.append(fn)
            
            f = fopen(fn, "wb")
            for i in xrange(fiStart, fiEnd):
                f.write(filesToBePreprocessed[i])
                f.write('\n')
            f.close()
            
            cmd.append('-i')
            cmd.append(fn)
            commands.append(cmd)
        
            if parseErrorFiles is not None:
                en = make_temp_filename(tempFileSeed, self.__syscnv)
                parseErrorFiles.append(en)
                tempFiles.append(en)
                cmd.append("--parseerrors=%s" % en)
            
        if verbose:
            progressBar = utility.ProgressReporter(len(commands))
        else:
            progressBar = utility.ProgressReporter(0)
        
        doneCount = 0
        for index, result in threadingutil.multithreading_iter(invoke_subprocess, commands, maxWorkerThreads):
            if result != 0:
                raise RuntimeError, "error in invocation of subprocess"
            doneCount += 1
            #progressBar.proceed(doneCount)
        
        if parseErrorFiles is not None:
            for en in parseErrorFiles:
                f = fopen(en, "r")
                if not f:
                    print >> sys.stderr, "error: can't open a temporary file '%s'" % en
                    sys.exit(2)
                parseErrorFiles.append(f.readlines())
                f.close()
                
        for fn in tempFiles:
            remove_file_neglecting_error(fn)
                
        progressBar.done()
    
    def __preprocess_files(self,
            filesToBePreprocessed, extensionStr, preprocessModule, 
            tempFileSeed = None, options = list(), verbose = False, 
            parseErrorFiles = None):
        filesToBePreprocessed = list(filesToBePreprocessed)
            
        encodingName = None
        preprocessorOptions = None
        prepDirs = list()
        for k, v in options:
            if k == '-c':
                encodingName = v
            elif k == '-r':
                preprocessorOptions = v
            elif k == '-n':
                prepDirs.append(v)
        
        cnv = easytorq.ICUConverter()
        if encodingName:
            cnv.setencoding(encodingName)
        
        prep = preprocessModule.getpreprocessor()
        if preprocessorOptions:
            prep.setoptions(preprocessorOptions)
        
        if verbose:
            progressBar = utility.ProgressReporter(len(filesToBePreprocessed))
        else:
            progressBar = utility.ProgressReporter(0)
        
        if len(filesToBePreprocessed) > 0:
            prefetch = FileReader(filesToBePreprocessed[0])
            prefetch.start()
        else:
            prefetch = None
    
        for i in xrange(len(filesToBePreprocessed)):
            if i + 1 < len(filesToBePreprocessed): 
                nextPrefetch = FileReader(filesToBePreprocessed[i + 1])
                nextPrefetch.start()
            else:
                nextPrefetch = None
            prefetch.join()
            preprocessedFname = prefetch.filename + extensionStr
            #print "prepDirs=", prepDirs #debug
            #print "preprocessedFname=", preprocessedFname #debug
            preprocessedFname = to_filename_in_prepdir(preprocessedFname, prepDirs)
            #print "preprocessedFname=", preprocessedFname #debug
            if prefetch.error is not None:
                print >> sys.stderr, "warning: not found file '%s'" % prefetch.filename
            else:
                try:
                    strUtf8 = cnv.decode(prefetch.content)
                except TypeError:
                    print >> sys.stderr, "error: invalid string (wrong character encoding?) in file '%s'" % prefetch.filename
                    raise
                parseResult = None
                try:
                    parseResult = prep.parse(strUtf8)
                except ValueError, e:
                    if parseErrorFiles is not None:
                        parseErrorFiles.append(prefetch.filename)
                    else:
                        print >> sys.stderr, "error: failure to parse file '%s'" % prefetch.filename
                        raise e
                if parseResult is not None:
                    preorocessedFnameTemp = preprocessedFname + "-temp"
                    try:
                        f = fopen(preorocessedFnameTemp, "wb")
                    except IOError:
                        try:
                            d = os_path_split(preprocessedFname, self.__syscnv)[0]
                            os.makedirs(d) 
                        except:
                            # Rarely, another process makes the directory while this process is trying to make the directory.
                            # In such case, the above mkdirs() fails, since the directory already exists.
                            if not os.path.exists(d):
                                raise # the directory does not exist and this process fails to make the directory
                        f = fopen(preorocessedFnameTemp, "wb")
                    try:
                        f.write(parseResult)
                        f.close()
                        rename_file(preorocessedFnameTemp, preprocessedFname) # rarely causes an error, the reason is unknown
                    except EnvironmentError, e:
                        print >> sys.stderr, "debug info: preorocessedFnameTemp=%s preprocessedFname=%s" % ( preorocessedFnameTemp, preprocessedFname )
                        remove_file_neglecting_error(preorocessedFnameTemp)
                        raise e
                progressBar.proceed(i + 1)
            prefetch = nextPrefetch
        progressBar.done()

if __name__ == '__main__':
    __theMain(__mlu).main(sys.argv)

