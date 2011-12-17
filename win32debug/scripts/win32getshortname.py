import ctypes

__GetShortPathName = ctypes.windll.kernel32.GetShortPathNameW

def getShortPathName(name):
    uname = unicode(name, 'cp932')
    buf = ctypes.create_unicode_buffer(32 * 1000)
    if __GetShortPathName(uname, ctypes.byref(buf), ctypes.sizeof(buf)):
        return buf.value.encode('cp932')
    else:
        return None

if __name__ == '__main__':
    import os
    import os.path
    
    p = r'C:\12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456\.ccfxprepdir'    
    fname = "hoge.hoge.hoge.hoge.hoge.hoge.hoge.hoge.hoge.hoge.txt"
    
    if not os.path.exists(p):
        os.makedirs(p)
    
    sp = getShortPathName(p)
    fp = os.path.join(p, fname)
    sfp = os.path.join(sp, fname)
    
    print "length of original file path = %d" % len(fp)
    print "length of shorter file path = %d" % len(sfp)
    
    f = file(sfp, "wb")
    print >> f, "hoge, hoge"
    f.close()

