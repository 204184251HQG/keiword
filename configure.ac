#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([klib], [0.1], [204184251@qq.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/log/process_bar.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_TYPE_INT32_T
# AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([
Makefile
src/Makefile
test/Makefile
])

AC_OUTPUT
