import Queue
import threading

class __Worker(threading.Thread):
    def __init__(self, func, argumentque, resultque):
        threading.Thread.__init__(self)
        self.func = func
        self.argumentque = argumentque
        self.resultque = resultque
    
    def run(self):
        while True:
            indexAndArgs = self.argumentque.get()
            if indexAndArgs is None:
                return
            index, args = indexAndArgs
            r = self.func(*args)
            self.resultque.put( ( index, r ) )

def multithreading_iter(func, argumentsgen, numworker):
    maxIndex = 0
    inputs = Queue.Queue()
    for indexAndArgs in enumerate(argumentsgen):
        inputs.put(indexAndArgs)
        maxIndex = indexAndArgs[0]
    for _ in xrange(numworker):
        inputs.put(None)
    outputs = Queue.Queue()
    
    workers = list()
    for i in xrange(numworker):
        th = __Worker(func, inputs, outputs)
        th.start()
        workers.append(th)
    
    for _ in xrange(maxIndex + 1):
        index, result = outputs.get()
        yield index, result
    
    for th in workers:
        th.join()

