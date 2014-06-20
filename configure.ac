#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ico-uxf-HomeScreen-sample-app], [0.0.1], [https://BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([1.11.1 -Wall -Werror foreign no-dist-gzip dist-xz silent-rules])
AC_CONFIG_SRCDIR([ico-app-vicsample/src/vicsample.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Checks for libraries.
PKG_PROG_PKG_CONFIG()
 
PKG_CHECK_MODULES([EINA], [eina])
PKG_CHECK_MODULES([EVAS], [evas])
PKG_CHECK_MODULES([ECORE], [ecore])
PKG_CHECK_MODULES([EDBUS], [edbus])
PKG_CHECK_MODULES([EDJE], [edje])
PKG_CHECK_MODULES([ELEM], [elementary])
EFL_CFLAGS="$EINA_CFLAGS $EVAS_CFLAGS $ECORE_CFLAGS $EDBUS_CFLAGS $EDJE_CFLAGS $ELEM_CFLAGS"
EFL_LIBS="$EINA_LIBS $EVAS_LIBS $ECORE_LIBS $EDBUS_LIBS $EDJE_LIBS $ELEM_LIBS"
AC_SUBST(EFL_CFLAGS)
AC_SUBST(EFL_LIBS)
 
PKG_CHECK_MODULES([AUL], [aul])
PKG_CHECK_MODULES([BUNDLE], [bundle])
AUL_CFLAGS="$AUL_CFLAGS $BUNDLE_CFLAGS"
AUL_LIBS="$AUL_LIBS $BUNDLE_CFLAGS"
AC_SUBST(AUL_CFLAGS)
AC_SUBST(AUL_LIBS)
 
 
if test "x$GCC" = "xyes"; then
	my_common_gcc_flags="-Wall -Wextra -Wno-unused-parameter \
-Wno-missing-field-initializers -g -fvisibility=hidden"
	GCC_CFLAGS="$my_common_gcc_flags \
-Wstrict-prototypes -Wmissing-prototypes"
	GCC_CXXFLAGS="-Wall -Wextra -Wno-unused-parameter \
-Wno-missing-field-initializers -g"
fi
 
AC_SUBST(GCC_CFLAGS)
AC_SUBST(GCC_CXXFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset sqrt strcasecmp strncasecmp strtol tzset])
AC_CONFIG_FILES([Makefile
                 ico-app-soundsample/src/Makefile
                 ico-app-vicsample/src/Makefile])
AC_OUTPUT
