lib_LTLIBRARIES = libgmsk.la
libgmsk_la_SOURCES = gmsk.h gmskinternal.h gmskmain.c gmskeditor.c \
                     gmskselect.c gmsknavbar.c gmsksaveload.c gmskmacros.c

libgmsk_la_LIBADD = $(GLIB_LIBS) $(PANGOCAIRO_LIBS) ../msk0/libmsk0.la
libgmsk_la_CFLAGS = $(GLIB_CFLAGS) $(PANGOCAIRO_CFLAGS) -I..
libgmsk_la_LDFLAGS = -no-undefined
