AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

bin_PROGRAMS = \
	ccfx/ccfx \
	picosel/picosel

pyexec_LTLIBRARIES = torq/pyeasytorq/easytorq.la

lib_LTLIBRARIES = ccfx/CCFinderXLib/lib_CCFinderXLib.la

common_CPPFLAGS = \
	-DOS_UBUNTU \
	-DLITTLE_ENDIAN \
	-DCXLM_NO_ENC \
	-DCODE_CONVERSION_SUPPORT \
	-DWSTRING_CONVERSION_SUPPORT \
	-DREQUIRE_RNR

common_LIBADD = -licule -licutu -licuio -licuuc -liculx -licudata -licui18n \
	-lboost_thread-mt

picosel_picosel_CPPFLAGS = $(common_CPPFLAGS) -O2 -fpermissive

picosel_picosel_SOURCES = \
	picosel/picosel.cpp \
	common/unportable.cpp

ccfx_ccfx_SOURCES = \
	common/base64encoder.cpp \
	common/bitvector.cpp \
	common/unportable.cpp \
	common/utf8support.cpp \
	ccfx/ccfx.cpp \
	ccfx/ccfxcommon.cpp \
	ccfx/prettyprintermain.cpp \
	ccfx/rawclonepairdata.cpp \
	ccfx/ccfxconstants.cpp

ccfx_ccfx_CPPFLAGS = $(common_CPPFLAGS) -O2 -fpermissive
ccfx_ccfx_LDFLAGS = $(common_LIBADD)

torq_pyeasytorq_easytorq_la_CPPFLAGS = $(common_CPPFLAGS) \
	-O2 -fpermissive $(PYTHON_INCLUDES)

torq_pyeasytorq_easytorq_la_SOURCES = \
	torq/pyeasytorq/pyeasytorq.cpp \
	torq/easytorq/easytorq.cpp \
	torq/interpreter.cpp \
	torq/texttoken.cpp \
	torq/torqcommon.cpp \
	common/unportable.cpp \
	common/utf8support.cpp

torq_pyeasytorq_easytorq_la_LIBADD = $(common_LIBADD)
torq_pyeasytorq_easytorq_la_LDFLAGS = -avoid-version -module


ccfx_CCFinderXLib_lib_CCFinderXLib_la_CPPFLAGS = $(common_CPPFLAGS) \
	-O2 -fpermissive -I/usr/lib/jvm/java-6-openjdk/include/

ccfx_CCFinderXLib_lib_CCFinderXLib_la_SOURCES = \
	ccfx/CCFinderXLib/CCFinderXLib.cpp \
	common/unportable.cpp \
	common/base64encoder.cpp \
	common/utf8support.cpp \
	ccfx/ccfxcommon.cpp \
	ccfx/ccfxconstants.cpp

ccfx_CCFinderXLib_lib_CCFinderXLib_la_LDFLAGS = \
	-version-info $(CCFINDER_LIB_VER) \
	-release $(PACKAGE_VERSION)

ccfx_CCFinderXLib_lib_CCFinderXLib_la_LIBADD = $(common_LIBADD)
