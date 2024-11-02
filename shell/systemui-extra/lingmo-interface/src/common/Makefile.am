#Generata binary file 
lib_LTLIBRARIES  = liblingmo-print.la liblingmo-gsettings.la liblingmo-com4c.la liblingmo-com4cxx.la

#gcc -wall: displays all the errors and warning information when compiling
#gcc -g: add the debugging code when compiling 
COMM_CFS = -Wall -g

#Add the dependent source file for liblingmo-com4c.la
liblingmo_com4c_la_SOURCES = lingmo-com4c.c \
    lingmo-ini.c \
	$(NULL)

#Additional C compiler flags
liblingmo_com4c_la_CFLAGS=  $(COMM_CFS) \
        -liniparser \
        $(NULL)

#Additional link objects
liblingmo_com4c_la_LDFLAGS= \
        $(NULL)

#Add the dependent source file for liblingmo-com4cxx.la
liblingmo_com4cxx_la_SOURCES = lingmo-com4cxx.cpp \
    lingmo-ini.c \
	$(NULL)

#Additional CXX compiler flags
liblingmo_com4cxx_la_CXXFLAGS=  $(COMM_CFS) \
        -liniparser \
        $(NULL)

#Additional C compiler flags
liblingmo_com4cxx_la_CFLAGS=  $(COMM_CFS) \
        $(NULL)

#Additional link objects
liblingmo_com4cxx_la_LDFLAGS= \
        $(NULL)

#Add the dependent source file for liblingmo-print.la
liblingmo_print_la_SOURCES = lingmo-print.c \
	$(NULL)

#Add the dependent source file for liblingmo-gsettings.la
liblingmo_gsettings_la_SOURCES = lingmo-gsettings-set.c \
	$(NULL)

#The header files that need to be installed
include_HEADERS= \
        lingmo-com4c.h \
	lingmo-com4cxx.h \
	lingmo-print.h \
	lingmo-gsettings-set.h \
	$(NULL)

#Additional C compiler flags
liblingmo_gsettings_la_CFLAGS=  $(COMM_CFS) \
        $(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
        $(NULL)

#Additional link objects
liblingmo_gsettings_la_LDFLAGS= \
        $(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
        $(NULL)
