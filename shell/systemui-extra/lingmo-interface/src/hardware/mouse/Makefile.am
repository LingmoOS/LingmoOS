COMM_PRINT= -L$(top_srcdir)/src/common -llingmo-print
COMM_GSETTINGS= -L$(top_srcdir)/src/common -llingmo-gsettings

#generate the .c and .h file which used by client and server side by XML file
#$(top_srcdir) represents the top-level directory of the project
DESC_XML = $(top_srcdir)/data/cn.lingmoos.mouse.xml
GENERATED_CH = mouse-generated

mouse-generated.h mouse-generated.c: $(DESC_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_CH) $(DESC_XML)

#Generata binary file 
lib_LTLIBRARIES  = liblingmo-mouseclient.la

#gcc -wall: displays all the errors and warning information when compiling
#gcc -g: add the debugging code when compiling 
COMM_CFS = -Wall -g

#Add the dependent source file for libmouseclient.la
#mouse-generated.c must be front of lingmo-mouse-interface.c
liblingmo_mouseclient_la_SOURCES = mouse-generated.c lingmo-mouse-interface.c \
	$(NULL)

#The header files that need to be installed
include_HEADERS= lingmo-mouse-interface.h \
	$(NULL)

#Additional C compiler flags
liblingmo_mouseclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_mouseclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Generata binary file
bin_PROGRAMS = lingmo-mouseserver

#Add the dependent source file for mouseserver
lingmo_mouseserver_SOURCES = \
	mouse-generated.h mouse-generated.c \
	lingmo-mouse-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_mouseserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_mouseserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)
