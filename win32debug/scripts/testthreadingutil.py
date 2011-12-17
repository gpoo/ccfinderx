
import threadingutil

import sys
import random
import time

random.seed(0)

def f(v): # this function must be declared at global scope, in order to make it visible to subprocess.
    time.sleep(random.random() * 2.0)
    return v * v

if __name__ == '__main__':
    usage = "Usage: testthreadingutil.py [NUMWORKER [INPUTSIZE]]"
    
    numWorker = 4
    inputSize = 30
    
    if len(sys.argv) >= 2:
        if sys.argv[1] == "-h":
            print usage
            sys.exit(0)
        numWorker = int(sys.argv[1])
    if len(sys.argv) >= 3:
        inputSize = int(sys.argv[2])
    if len(sys.argv) >= 4:
        print usage
        sys.exit(1)
    
    def genargslist(size):
        for v in xrange(size):
            yield ( v, )
    
    t1 = time.time()
    
    #for index, result in threadingutil.multithreading_iter(f, [ args for args in genargslist(inputSize) ], numWorker):
    for index, result in threadingutil.multithreading_iter(f, genargslist(inputSize), numWorker):
        print "index = ", index, ", result = ", result
    
    print
    print "NUMWORKER = %d, INPUTSIZE = %d" % ( numWorker, inputSize )
    print "elapsed time: %g" % (time.time() - t1)
