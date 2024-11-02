COMM_PRINT= -L$(top_srcdir)/src/common -llingmo-print
COMM_GSETTINGS= -L$(top_srcdir)/src/common -llingmo-gsettings

#generate the .c and .h file which used by client and server side by XML file
#$(top_srcdir) represents the top-level directory of the project
DESC_XML = $(top_srcdir)/data/cn.lingmoos.touchpad.xml
GENERATED_CH = touchpad-generated

touchpad-generated.h touchpad-generated.c: $(DESC_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_CH) $(DESC_XML)

#Generata binary file 
lib_LTLIBRARIES  = liblingmo-touchpadclient.la

#gcc -wall: displays all the errors and warning information when compiling
#gcc -g: add the debugging code when compiling 
COMM_CFS = -Wall -g

#Add the dependent source file for libtouchpadclient.la
#touchpad-generated.c must be front of lingmo-touchpad-interface.c
liblingmo_touchpadclient_la_SOURCES = touchpad-generated.c lingmo-touchpad-interface.c \
	$(NULL)

#The header files that need to be installed
include_HEADERS= lingmo-touchpad-interface.h \
	$(NULL)

#Additional C compiler flags
liblingmo_touchpadclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_touchpadclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Generata binary file
bin_PROGRAMS = lingmo-touchpadserver

#Add the dependent source file for touchpadserver
lingmo_touchpadserver_SOURCES = \
	touchpad-generated.h touchpad-generated.c \
	lingmo-touchpad-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_touchpadserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_touchpadserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)
