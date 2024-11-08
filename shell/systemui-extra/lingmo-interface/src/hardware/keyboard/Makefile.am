COMM_PRINT= -L$(top_srcdir)/src/common -llingmo-print
COMM_GSETTINGS= -L$(top_srcdir)/src/common -llingmo-gsettings

#generate the .c and .h file which used by client and server side by XML file
#$(top_srcdir) represents the top-level directory of the project
GENE_XML = $(top_srcdir)/data/cn.lingmoos.xkbgeneral.xml
INTE_XML = $(top_srcdir)/data/cn.lingmoos.interface.xml
KEY_XML = $(top_srcdir)/data/cn.lingmoos.keyboard.xml

GENERATED_GENE = xkbgeneral-generated
GENERATED_INTE = interface-generated
GENERATED_KEY = keyboard-generated

xkbgeneral-generated.h xkbgeneral-generated.c: $(GENE_XML)
	gdbus-codegen --interface-prefix cn.lingmoos --generate-c-code $(GENERATED_GENE) $(GENE_XML)

interface-generated.h interface-generated.c: $(INTE_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_INTE) $(INTE_XML)

keyboard-generated.h keyboard-generated.c: $(KEY_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_KEY) $(KEY_XML)


#Generata binary file 
lib_LTLIBRARIES  = liblingmo-interfaceclient.la liblingmo-keyboardclient.la

#gcc -wall: displays all the errors and warning information when compiling
#gcc -g: add the debugging code when compiling 
COMM_CFS = -Wall -g

#The header files that need to be installed
include_HEADERS= lingmo-interface-interface.h \
    lingmo-keyboard-interface.h \
    $(NULL)

#Add the dependent source file for libinterfaceclient.la
#interface-generated.c must be front of lingmo-interface-interface.c
liblingmo_interfaceclient_la_SOURCES = interface-generated.c lingmo-interface-interface.c \
	$(NULL)

#Additional C compiler flags
liblingmo_interfaceclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_interfaceclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for libkeyboardclient.la
#keyboard-generated.c must be front of lingmo-keyboard-interface.c
liblingmo_keyboardclient_la_SOURCES = keyboard-generated.c lingmo-keyboard-interface.c \
	$(NULL)

#Additional C compiler flags
liblingmo_keyboardclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_keyboardclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Generata binary file
bin_PROGRAMS =lingmo-xkbgeneralserver lingmo-interfaceserver lingmo-keyboardserver

#Add the dependent source file for xkbgeneralserver
lingmo_xkbgeneralserver_SOURCES = \
	xkbgeneral-generated.h xkbgeneral-generated.c \
	lingmo-xkbgeneral-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_xkbgeneralserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_xkbgeneralserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for interfaceserver
lingmo_interfaceserver_SOURCES = \
	interface-generated.h interface-generated.c \
	lingmo-interface-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_interfaceserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_interfaceserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for keyboardserver
lingmo_keyboardserver_SOURCES = \
	keyboard-generated.h keyboard-generated.c \
	lingmo-keyboard-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_keyboardserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_keyboardserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)
