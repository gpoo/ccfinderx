AC_PREREQ(2.5.2)
AC_INIT(ccfx/ccfx.cpp,10.2.8)

AM_INIT_AUTOMAKE(1.8)

AC_PROG_CXX

AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

PYTHON_INCLUDES=`python-config --cflags`
AC_SUBST(PYTHON_INCLUDES)
AM_PATH_PYTHON

# Check for Boost
AX_BOOST_BASE

# Check for JNI
AX_JNI_INCLUDE_DIR
for JNI_INCLUDE_DIR in $JNI_INCLUDE_DIRS
do
    CPPFLAGS="$CPPFLAGS -I$JNI_INCLUDE_DIR"
done

# Check for ICU
AC_CHECK_PROG(ICU_CHECK, [icu-config], [yes])
if test x"$ICU_CHECK" != x"yes" ; then
    AC_MSG_ERROR([ICU (libicu) library package required but not found.])
fi
ICU_LIBS=`icu-config --ldflags --ldflags-icuio --ldflags-layout`
AC_SUBST(ICU_LIBS)

LT_INIT
CCFINDER_LIB_VER="10:2:8"
AC_SUBST(CCFINDER_LIB_VER)

AC_OUTPUT(
  Makefile
)
