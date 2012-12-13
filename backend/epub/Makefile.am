INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libdocument 		\
	-DGNOMELOCALEDIR=\"$(datadir)/locale\"	\
	-DEVINCE_COMPILATION			\
	$(BACKEND_CFLAGS)			\
	$(LIBGEPUB_CFLAGS)			\
	$(LIB_CFLAGS)				\
	$(WARN_CFLAGS)				\
	$(DISABLE_DEPRECATED)

backend_LTLIBRARIES = libepubdocument.la

libepubdocument_la_SOURCES = \
	epub-document.c      \
	epub-document.h

libepubdocument_la_LDFLAGS = $(BACKEND_LIBTOOL_FLAGS)
libepubdocument_la_LIBADD =				\
	$(top_builddir)/libdocument/libevdocument3.la	\
	$(BACKEND_LIBS)					\
	$(LIBGEPUB_LIBS)				\
	$(LIB_LIBS)

backend_in_files = epubdocument.evince-backend.in
backend_DATA = $(backend_in_files:.evince-backend.in=.evince-backend)

EXTRA_DIST = $(backend_in_files)

CLEANFILES = $(backend_DATA)

@EV_INTLTOOL_EVINCE_BACKEND_RULE@

-include $(top_srcdir)/git.mk
