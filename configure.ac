#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/events.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for packages.
PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GTK, [gtk+-2.0])
PKG_CHECK_MODULES(GMODULE, [gmodule-2.0])
PKG_CHECK_MODULES(PORTAUDIO, [portaudio-2.0])

# Checks for libraries.
AC_CHECK_LIB([jackmp], [jack_client_open])
#AC_CHECK_LIB([winmm], [waveOutOpen])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_VOLATILE

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/msk0/Makefile
                 src/mod-winmm/Makefile])
#                src/mod-jack/Makefile
#                src/mod-portaudio/Makefile
                 
AC_OUTPUT
