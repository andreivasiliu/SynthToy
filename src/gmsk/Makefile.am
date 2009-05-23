lib_LTLIBRARIES = libgmsk.la
libgmsk_la_SOURCES = gmsk.h gmskinternal.h gmskmain.o gmskeditor.o \
                     gmskselect.o gmsknavbar.o gmsksaveload.o gmskmacros.o

libgmsk_la_LIBADD = $(GLIB_LIBS) $(PANGOCAIRO_LIBS) -lm
libgmsk_la_CFLAGS = $(GLIB_CFLAGS) $(PANGOCAIRO_CFLAGS)
libgmsk_la_LDFLAGS = -no-undefined
