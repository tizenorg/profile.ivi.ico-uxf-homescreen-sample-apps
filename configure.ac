#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ico-uxf-HomeScreen-sample-app], [0.0.1], [https://BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([1.11.1 -Wall -Werror foreign silent-rules])
AC_CONFIG_SRCDIR([ico-app-vicsample/src/vicsample.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

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
                 ico-app-samplenavi/src/Makefile
                 ico-app-soundsample/src/Makefile
                 ico-app-vicsample/src/Makefile])

AC_OUTPUT
