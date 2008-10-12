bin_PROGRAMS = first
pkgdata_DATA = first.ui
EXTRA_DIST = first.glade
CLEANFILES = first.ui

first_SOURCES = first.c events.c process.c virkb.c mskeditor.c header.h
first_LDADD = msk0/libmsk0.la $(GTK_LIBS)
first_LDFLAGS = -static -rdynamic
first_CFLAGS = $(GTK_CFLAGS)


SUBDIRS = msk0

if HAVE_WINMM
  SUBDIRS += mod-winmm
  first_LDADD += mod-winmm/modwinmm.la
endif
if HAVE_JACK
  SUBDIRS += mod-jack
  first_LDADD += mod-jack/modjack.la
endif
if HAVE_PORTAUDIO
  SUBDIRS += mod-portaudio
endif


first.ui: $(srcdir)/first.glade
	gtk-builder-convert $(srcdir)/first.glade first.ui
