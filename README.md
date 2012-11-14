CCFinderX
=========

This is a clone of [CCFinderX][1] that has the settings to build with autoconf on a Linux machine.

The autoconf setting is not finished (it does not pass `make distcheck`), but it is something to start with. The process to build `ccfinderx` is:

    $ libtoolize
    $ aclocal -I m4 --install
    $ autoconf
    $ automake --foreign --add-missing
    $ configure
    $ make

If there is an error in make `ccfinderx_CCFinderX.h`, then is necessary to generate it via:

    $ cd GemX
    $ make ccfinderx_CCFinderX_h

To build GemX (the graphical user interface):

    $ cd GemX
    $ make

  [1]: http://www.ccfinder.net/ccfinderxos.html
