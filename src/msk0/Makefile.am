lib_LTLIBRARIES = libmsk0.la
libmsk0_la_SOURCES = mskmodule.c mskworld.c mskcontainer.c mskconstant.c \
                     mskoscillator.c mskaddmul.c msk0.h mskinternal.h

libmsk0_la_LIBADD = $(GLIB_LIBS) -lm
libmsk0_la_CFLAGS = $(GLIB_CFLAGS)
libmsk0_la_LDFLAGS = -no-undefined
