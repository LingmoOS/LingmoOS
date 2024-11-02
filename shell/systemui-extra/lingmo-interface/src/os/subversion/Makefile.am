lib_LTLIBRARIES  = liblingmo-subversion.la

COMM_CFS = -Wall -g -fPIC

liblingmo_subversion_la_SOURCES = lingmo-version-interface.cpp \
	$(NULL)

include_HEADERS= \
	lingmo-version-interface.h \
	$(NULL)

liblingmo_subversion_la_CPPFLAGS= $(COMM_CFS) \
	$(QT5CORE_CFLAGS) \
	$(NULL)

liblingmo_subversion_la_LDFLAGS= \
	$(QT5CORE_LIBS) \
	$(NULL)
