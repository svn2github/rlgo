dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([rlgo/RlAgent.cpp])
AM_CONFIG_HEADER([utils/RlConfig.h])
AC_CONFIG_FILES([Makefile utils/Makefile rlgo/Makefile policies/Makefile features/Makefile learning/Makefile rlgomain/Makefile unittest/Makefile input/Makefile scripts/Makefile settings/Makefile])
AC_CONFIG_FILES([scripts/getprogram.sh scripts/match.sh scripts/analyze-match.sh], [chmod +x scripts/getprogram.sh scripts/match.sh scripts/analyze-match.sh])
AM_INIT_AUTOMAKE([rlgo], [2.5])
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_PROG_RANLIB
AX_BOOST_BASE([1.33.1])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_UNIT_TEST_FRAMEWORK

AC_ARG_WITH([fuego-dir], 
    [AS_HELP_STRING(
        [--with-fuego-dir=fuego_directory],
        [specify location of fuego distribution])],
    [AC_SUBST(
        [fuego_dir],
        [$withval])], 
    [AC_MSG_ERROR(
        ["Must specify location of fuego (version 0.4) using --with-fuego-dir"])]
)
          
dnl Check for optional use of gogui-twogtp
AC_CHECK_PROGS([twogtp], [gogui-twogtp twogtp], [not_found])
if test "$twogtp" = "not_found"
then
    AC_MSG_WARN(
        ["gogui-twogtp not found: won't be able to run experiment scripts"])
fi

dnl Check for optional use of gnugo
AC_CHECK_PROGS([gnugo], [gnugo], [not_found])
if test "$gnugo" = "not_found"
then
    AC_MSG_WARN(
        ["gnugo not found: won't be able to run experiment scripts"])
fi  

dnl Check for optional use of fuego program
AC_CHECK_PROGS([fuego], [fuego], [not_found])
if test "$fuego" = "not_found"
then
    AC_MSG_WARN(
        ["fuego program not found: won't be able to run experiments against fuego"])
fi

dnl Check for optional use of gnuplot program
AC_CHECK_PROGS([gnuplot], [gnuplot], [not_found])
if test "$gnuplot" = "not_found"
then
    AC_MSG_WARN(
        ["gnuplot program not found: won't be able to plot experiments"])
fi

dnl Check for optional use of bayeselo program
AC_CHECK_PROGS([bayeselo], [bayeselo], [not_found])
if test "$bayeselo" = "not_found"
then
    AC_MSG_WARN(
        ["bayeselo program not found: won't be able to analyze experiments"])
fi

AC_ARG_ENABLE([optimize],
    AS_HELP_STRING(
        [--enable-optimize], 
        [set CXXFLAGS to -O3 -g (default is yes)]),
    [optimize=$enableval],
    [optimize=yes])
if test "x$optimize" = "xyes"
then
	CXXFLAGS="-O3 -g"
else
	CXXFLAGS="-g"
fi

AC_ARG_ENABLE([assert],
	      AS_HELP_STRING([--enable-assert], [enable assertions (default is no)]),
	      [assert=$enableval],
	      [assert=no])
if test "x$assert" = "xno"
then
	CXXFLAGS="$CXXFLAGS -DNDEBUG"
else
	CXXFLAGS="$CXXFLAGS"
fi

AX_CXXFLAGS_WARN_ALL
AX_CXXFLAGS_GCC_OPTION(-Wextra)

AC_OUTPUT
