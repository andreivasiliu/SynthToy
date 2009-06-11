lib_LTLIBRARIES = libmsk0.la
libmsk0_la_SOURCES = msk0.h mskinternal.h mskworld.c mskcontainer.c \
                     mskmodule.c mskconstant.c mskoscillator.c mskaddmul.c \
                     mskfactory.c msksaveload.c mskadapter.c mskinstrument.c \
                     mskdelay.c mskfilter.c mskdistort.c

libmsk0_la_LIBADD = $(GLIB_LIBS) -lm
libmsk0_la_CFLAGS = $(GLIB_CFLAGS)
libmsk0_la_LDFLAGS = -no-undefined
