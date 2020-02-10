#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([keiworld], [0.1], [204184251@qq.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB

AC_ARG_ENABLE(keilib, [AS_HELP_STRING([--enable-keilib=@<:@yes/no@:>@],
                            [whether to use keilib [default=yes]])], [], [enable_keilib=yes])

AM_CONDITIONAL([KEILIB_OPT], [test "$enable_keilib" = yes])
KEILIB_DIR=
AM_COND_IF([KEILIB_OPT],[
    if test -d "$srcdir/keilib"; then
        AC_MSG_NOTICE(configuring keilib)
        AC_CONFIG_SUBDIRS([keilib])
        KEILIB_DIR=keilib
    fi
    ],
    [])
AC_SUBST([KEILIB_DIR])
# Checks for libraries.
AC_MSG_CHECKING(for BulletDynamics BulletCollision LinearMath support)
AC_LANG_SAVE
AC_LANG(C++)
SAVED_LDFLAGS=$LDFLAGS
SAVED_LIBS=$LIBS
SAVED_CXXFLAGS=$CXXFLAGS
BULLET_LIBS="-lBulletDynamics -lBulletCollision -lLinearMath"
BULLET_CXXFLAGS=" -I/usr/local/include/bullet"
LDFLAGS="$LDFLAGS"
LIBS="$LIBS $BULLET_LIBS"
CXXFLAGS="$CXXFLAGS $BULLET_CXXFLAGS"
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <btBulletDynamicsCommon.h>],
        [void *a = new btDefaultCollisionConfiguration();])],
    [TEST_LIBS="$TEST_LIBS $BULLET_LIBS"] [HAVE_BULLET=1], 
    [AC_MSG_ERROR([libbullet is not installed.]);
    LDFLAGS=$SAVED_LDFLAGS;
    LIBS=$SAVED_LIBS;
    CXXFLAGS=$SAVED_CXXFLAGS])

AC_LANG_RESTORE

AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.


# Config Makefiles
AC_CONFIG_FILES([
Makefile
src/Makefile
])



AC_OUTPUT
