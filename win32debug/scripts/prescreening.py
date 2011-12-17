#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import sys
import os
import tempfile
import getopt
import time
import subprocess

import ccfxconfig
import utility

MaskedFileRemark = "masked"
Exec_ = "exec:"

DEFAULT_RSI_LIMIT = 0.7
DEFAULT_LENGTH_LIMIT = 20000

def invoke_subprocess(args):
    subp = subprocess.Popen(args, shell=False, stdin=None, stdout=None, stderr=None)
    subp.wait()
    return subp.returncode

def extractCloneDataWithinFiles(cloneData, prep, encodingName, filelist, infos, verbose=False):
    cmdline = [ infos.ccfxPath, "d", prep, "-i", filelist, "-w", "w+f-g-", "-o", cloneData, 
            "--prescreening=LEN.gt.%d" % DEFAULT_LENGTH_LIMIT ]
    if encodingName:
        cmdline.extend([ "-c", encodingName ])
    if verbose: cmdline.append("-v")
    if verbose: 
        print >> sys.stderr, "%s %s" % (Exec_, " ".join(cmdline))
        sys.stderr.flush()
    return invoke_subprocess(cmdline)

def calcFileMetricValues(metricsData, cloneData, infos, verbose=False):
    cmdline = [ infos.ccfxPath, "m", cloneData, "-f", "-o", metricsData ]
    if verbose: 
        print >> sys.stderr, "%s %s" % (Exec_, " ".join(cmdline))
        sys.stderr.flush()
    return invoke_subprocess(cmdline)

def extractFiles(predicate, metricsData, infos, verbose=False):
    titleLineFields = "FID LEN CLN NBR RSA RSI CVR RNR".split(" ")
    lenTitleLineFields = len(titleLineFields)
    
    ids = set()
    for line in file(metricsData):
        fields = line.strip().split('\t')
        assert len(fields) == lenTitleLineFields
        #idStr, lenStr, clnStr, nbrStr, rsaStr, rsiStr, cvrStr, rnrStr = fields
        idStr = fields[0]
        if idStr == 'FID':
            assert fields == titleLineFields
            continue
        idValue = int(idStr)
        values = map(float, fields[1:])
        if predicate(idValue, values[0], values[1], values[4], values[6]):
            ids.add(idValue)
    return sorted(list(ids))

def writeMaskedFileList(output, maskedIDs, filelist, remarkString, verbose=False):
    maskedIDSet = set(maskedIDs[:])
    if output != "-":
        outputFile = file(output, "w")
    else:
        outputFile = sys.stdout
    outputFile_write = outputFile.write
    try:
        fileIDToPathTable = dict()
        largestFileID = 0
        for line in file(filelist):
            fields = line.strip().split('\t')
            lenFields = len(fields)
            if lenFields == 2 and fields[0] == ':':
                remarkStr = fields[0]
                outputFile_write(":\t%s\n" % remarkStr)
            else:
                if lenFields == 1:
                    fileID, filePath = largestFileID + 1, fields[0]
                elif lenFields == 2:
                    fileID, filePath = int(fields[0]), fields[1]
                if fileID < 0:
                    outputFile_write("error: wrong file ID\n")
                    return 2
                largestFileID = max(largestFileID, fileID)
                if fileID in fileIDToPathTable:
                    outputFile_write("error: file ID conflict\n")
                    return 2
                outputFile_write("%d\t%s\n" % (fileID, filePath))
                if fileID in maskedIDSet:
                    outputFile_write(":\t%s\n" % remarkString)
    finally:
        if output != "-":
            outputFile.close()
    return 0



def make_temp_filename(output, suffix):
    value = os.environ.get("CCFINDER_TEMPORARY_DIRECTORY")
    fdsc, fn = None, None
    if value != None:
        fdsc, fn = tempfile.mkstemp(suffix, dir=value)
    elif output:
        d = os.path.split(output)[0]
        fdsc, fn = tempfile.mkstemp(suffix, dir=d)
    else:
        fdsc, fn = tempfile.mkstemp(suffix)
    f = os.fdopen(fdsc)
    f.close()   
    return fn

def extractCloneDataWithMask(cloneData, prep, filelist, remarkString, infos, verbose=False):
    cmdline = [ infos.ccfxPath, "d", prep, "-i", tempFileList, "-o", output, "-mr", remarkString ]
    if verbose: 
        print >> sys.stderr, "%s %s" % (Exec_, string.join(cmdline, " "))
        sys.stderr.flush()
    return invoke_subprocess(cmdline)

if __name__ == '__main__':
    infos = ccfxconfig.Infos()
    
    if len(sys.argv) == 1 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
        print """Usage: prescreening PREP OPTIONS -i FILELIST
Option
  -c ENCODING: specify encoding.
  -m STRING: remark string to masked files.
  -o OUTPUT: output file.
  -p PRED: predicate on FID, LEN, CLN, RSI, RNR.
  -v: verbose.
"""
        sys.exit(0)
    
    verbose = False
    remarkString = MaskedFileRemark
    optionCloneDetection = False
    filelist = None
    output = None
    predicate = None
    preprocessScript = sys.argv[1]
    encodingName = None
    if preprocessScript not in infos.preprocessScripts:
        print >> sys.stderr, "error: invalid preprocess script"
        sys.exit(1)
    opts, args = getopt.gnu_getopt(sys.argv[2:], "i:o:m:p:vc:")
    for name, value in opts:
        if name == '-i':
            filelist = value
        elif name == '-o':
            output = value
        elif name == '-m':
            remarkString = value
        elif name == "-p":
            predicate = eval("lambda FID, LEN, CLN, RSI, RNR: " + value)
        elif name == '-v':
            verbose = True
        elif name == '-c':
            encodingName = value
    if filelist == None:
        print >> sys.stderr, "error: no input file"
        sys.exit(1)
    
    tempFiles = list()
    try:
        tempCloneData = make_temp_filename(output, ".ccfxd")
        tempFiles.append(tempCloneData)
        retcode = extractCloneDataWithinFiles(tempCloneData, preprocessScript, encodingName, filelist, infos, verbose)
        if retcode != 0: sys.exit(retcode)
        
        tempMetricsData = make_temp_filename(output, ".fm.tmp")
        tempFiles.append(tempMetricsData)
        retcode = calcFileMetricValues(tempMetricsData, tempCloneData, infos, verbose)
        if retcode != 0: sys.exit(retcode)
        
        if predicate == None:
            def predicate(FID, LEN, CLN, RSI, RNR): return RSI >= DEFAULT_RSI_LIMIT or LEN > DEFAULT_LENGTH_LIMIT
        maskedIDs = extractFiles(predicate, tempMetricsData, infos)

        if output == None:
            output = "-"
        retcode = writeMaskedFileList(output, maskedIDs, filelist, remarkString, verbose)
        if retcode != 0: sys.exit(retcode)
    finally:
        for f in tempFiles:
            try:
                os.remove(f)
            except:
                pass

    sys.exit(0)

