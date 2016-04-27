CCFinderX
=========

CCFinderX is a tool for detecting code clones from source code,  originally developed by Toshiro Kamiya.
It is a re-design a previous tool CCFinder, described in the paper *"[CCFinder: A Multi-Linguistic Token-based Code Clone Detection System for Large Scale Source Code][1]"*.

CCFindeX can extract code code clones correctly from source code even in cases where the names of variables habe changed.

Features of CCFinderX
---------------------

* Detect duplicated code written in Java, C, C++, COBOL, VisualBasic and C#.
* Multi-threadding for multi-core CPU
* AST-based preprocessing with an island-parser like parser.
* Search functions
* Support for programming languages in (as possible as) equal level.
* Users can adapt the tool to another programming languages or dialects.
* Analysis using metrics of code clone
* For interoperability with another tools, the tool can read/write data in TSV(tab separated values) text format.
* Code clone shaper as described in *"[On Software Maintenance Process Improvement Based On Code Clone Analysis][5]"*
* Support for parameterized match (P-match) as described in *"[On finding Duplication and Near-Duplication in Large Software System][4]"*.
* Interactive analysis with multiple views of GUI front-end GemX, a re-implementation of Gemini, described in *"[Gemini: Code Clone Analysis Tool][6]"*.


About this repository
---------------------

This is a clone of [CCFinderX][2] that has the settings to build with
autoconf on a Linux machine.

On Debian 8 (jessie), you need at least the following packages installed:
    # aptitude install libtool autoconf automake make build-essential autoconf-archive

You'll also need various Boost libraries, it may be easiest to just do:

    # aptitude install libboost-all-dev

Note that `autconf-archive` is important to get `AX_JNI_INCLUDE_DIR` and
`AX_BOOST_BASE` referenced in `configure.ac`.

After dependencies are installed, the process to build `ccfinderx` is:

    $ libtoolize
    $ aclocal -I m4 --install
    $ autoconf
    $ automake --foreign --add-missing
    $ ./configure
    $ make

`ccfinderx` requires the development packages for Python, Boost, JNI (Java SDK), and ICU.

If there is an error in make `ccfinderx_CCFinderX.h`, then is necessary to generate it via:

    $ cd GemX
    $ make ccfinderx_CCFinderX_h

To build GemX (the graphical user interface):

    $ cd GemX
    $ make

License
-------

This fork was made of CCFinder distributed under MIT License since January 25, 2010 (as declared in [CCFinder official web page][3].

  [1]: http://dx.doi.org/10.1109/TSE.2002.1019480
  [2]: http://www.ccfinder.net/ccfinderxos.html
  [3]: http://www.ccfinder.net/index.html
  [4]: http://dx.doi.org/10.1109/WCRE.1995.514697
  [5]: http://link.springer.com/chapter/10.1007/3-540-36209-6_17
  [6]: http://sel.ist.osaka-u.ac.jp/~lab-db/betuzuri/archive/386/386.pdf
