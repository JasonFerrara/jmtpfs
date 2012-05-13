dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(jmtpfs, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()


AC_PROG_CXX

CXXFLAGS="$CXXFLAGS -std=c++0x"

PKG_CHECK_MODULES(MTP, libmtp >= 1.1.0)
AC_SUBST(MTP_CFLAGS)
AC_SUBST(MTP_LIBS)

PKG_CHECK_MODULES(FUSE, fuse >= 2.6)
AC_SUBST(FUSE_CFLAGS)
AC_SUBST(FUSE_LIBS)

AC_CHECK_LIB([magic],[magic_open])
AC_CHECK_HEADERS([magic.h])


AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

