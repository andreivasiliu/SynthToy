#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT(synthtoy, [0.0])
AC_CONFIG_SRCDIR([src/events.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_ARG_WITH([winmm],
  [AC_HELP_STRING([--with-winmm],
    [enable support for Windows Multimedia API])],
  [], [with_winmm=check])
AM_CONDITIONAL(HAVE_WINMM, false)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Make sure these macros exists.
m4_pattern_forbid([^PKG_])
m4_pattern_forbid([^MSK_])

# Checks for required packages.
PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(GMODULE, [gmodule-2.0])
PKG_CHECK_MODULES(GTK, [gtk+-2.0])
PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])

# Checks for optional pakages. The macro is defined in acinclude.m4.
MSK_CHECK_OPTIONAL_MODULE([portaudio], [PortAudio], [PORTAUDIO],
  [portaudio-2.0])
MSK_CHECK_OPTIONAL_MODULE([jack], [JACK], [JACK], [jack])

# Checks for libraries.
#AC_CHECK_LIB([m], [sinf])
#AC_CHECK_LIB([jackmp], [jack_client_open])
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
                 src/mod-winmm/Makefile
                 src/mod-jack/Makefile
                 src/mod-portaudio/Makefile])
                 
AC_OUTPUT


# Echo some status.
echo
echo "Status:"
echo -n "  portaudio support... "
AS_IF([test "$have_portaudio" == "yes"], [echo "yes"], [echo "no"])
echo -n "  jack support... "
AS_IF([test "$have_jack" == "yes"], [echo "yes"], [echo "no"])
echo -n "  winmm support... "
AS_IF([test "$have_winmm" == "yes"], [echo "yes"], [echo "no"])
echo