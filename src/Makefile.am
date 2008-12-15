bin_PROGRAMS = synthtoy
pkgdata_DATA = first.ui
EXTRA_DIST = first.glade
CLEANFILES = first.ui

synthtoy_SOURCES = first.c events.c process.c virkb.c gmskeditor.c \
		   gmskmenu.c gmsknavbar.c header.h
synthtoy_LDADD = msk0/libmsk0.la $(GTK_LIBS) $(GTHREAD_LIBS)
synthtoy_LDFLAGS = -static -rdynamic
synthtoy_CFLAGS = $(GTK_CFLAGS) $(GTHREAD_CFLAGS)


SUBDIRS = msk0

if HAVE_WINMM
  SUBDIRS += mod-winmm
  synthtoy_LDADD += mod-winmm/modwinmm.la
endif
if HAVE_JACK
  SUBDIRS += mod-jack
  synthtoy_LDADD += mod-jack/modjack.la
endif
if HAVE_PORTAUDIO
  SUBDIRS += mod-portaudio
endif


first.ui: $(srcdir)/first.glade
	gtk-builder-convert $(srcdir)/first.glade first.ui
