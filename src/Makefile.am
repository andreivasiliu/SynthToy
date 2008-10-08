bin_PROGRAMS = first
pkgdata_DATA = first.ui
EXTRA_DIST = first.glade
CLEANFILES = first.ui

first_SOURCES = first.c events.c process.c virkb.c mskeditor.c header.h
first_LDADD = msk0/libmsk0.la mod-winmm/modwinmm.la $(GTK_LIBS)
first_CFLAGS = $(GTK_CFLAGS)


SUBDIRS = msk0 mod-winmm

first.ui: $(srcdir)/first.glade
	gtk-builder-convert $(srcdir)/first.glade first.ui
