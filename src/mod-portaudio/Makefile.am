
lib_minedir = $(libdir)/mine
lib_mine_LTLIBRARIES = simple.la
simple_la_SOURCES = simple.c
simple_la_LDFLAGS = -no-undefined -module $(GLIB_LIBS) $(PORTAUDIO_LIBS)
simple_la_CFLAGS = $(GLIB_CFLAGS) $(PORTAUDIO_CFLAGS)

bin_PROGRAMS = user
user_SOURCES = user.c
user_LDFLAGS = -dlopen $(GMODULE_LIBS)
user_CFLAGS = $(GMODULE_CFLAGS)
