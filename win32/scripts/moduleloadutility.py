import imp

class ModuleLoadUtility(object):
    def __init__(self):
        self.__alreadyLoadedPackage = dict() # dotName -> module
        
    
    def load(self, moduleDotName):
        fields = moduleDotName.split(".")
        prevM = None
        for i, n in enumerate(fields):
            dotName = ".".join(fields[0:i + 1])
            alreadyLoaded = self.__alreadyLoadedPackage.get(dotName, None)
            if alreadyLoaded:
                m = alreadyLoaded
            else:
                if prevM:
                    fileObj, pathName, desc = imp.find_module(n, prevM.__path__)
                else:
                    fileObj, pathName, desc = imp.find_module(n)
                m = imp.load_module(n, fileObj, pathName, desc)
                self.__alreadyLoadedPackage[dotName] = m
            prevM = m
        return m


