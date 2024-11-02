COMM_PRINT= -L$(top_srcdir)/src/common -llingmo-print
COMM_GSETTINGS= -L$(top_srcdir)/src/common -llingmo-gsettings

#generate the .c and .h file which used by client and server side by XML file
#$(top_srcdir) represents the top-level directory of the project
POWER_XML = $(top_srcdir)/data/cn.lingmoos.power.xml
GENERATED_POW = power-generated

SCREEN_XML = $(top_srcdir)/data/cn.lingmoos.screensaver.xml
GENERATED_SCREEN = screensaver-generated

SESS_XML = $(top_srcdir)/data/cn.lingmoos.session.xml
GENERATED_SESS = session-generated

power-generated.h power-generated.c: $(POWER_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_POW) $(POWER_XML)

screensaver-generated.h screensaver-generated.c: $(SCREEN_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_SCREEN) $(SCREEN_XML)

session-generated.h session-generated.c: $(SESS_XML)
	gdbus-codegen --interface-prefix cn.lingmoos. --generate-c-code $(GENERATED_SESS) $(SESS_XML)

#Generata binary file 
lib_LTLIBRARIES  = liblingmo-powerclient.la liblingmo-screensaverclient.la liblingmo-sessionclient.la

#gcc -wall: displays all the errors and warning information when compiling
#gcc -g: add the debugging code when compiling 
COMM_CFS = -Wall -g

#Add the dependent source file for libpowerclient.la
#power-generated.c must be front of lingmo-power-interface.c
liblingmo_powerclient_la_SOURCES = power-generated.c lingmo-power-interface.c \
	$(NULL)

#The header files that need to be installed
include_HEADERS= lingmo-power-interface.h \
	lingmo-screensaver-interface.h \
	lingmo-session-interface.h \
	$(NULL)

#Additional C compiler flags
liblingmo_powerclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_powerclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for libscreensaverclient.la
#screensaver-generated.c must be front of lingmo-screensaver-interface.c
liblingmo_screensaverclient_la_SOURCES = screensaver-generated.c lingmo-screensaver-interface.c \
	$(NULL)

#Additional C compiler flags
liblingmo_screensaverclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_screensaverclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for libsessionclient.la
#session-generated.c must be front of lingmo-session-interface.c
liblingmo_sessionclient_la_SOURCES = session-generated.c lingmo-session-interface.c \
	$(NULL)

#Additional C compiler flags
liblingmo_sessionclient_la_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
liblingmo_sessionclient_la_LDFLAGS= $(COMM_PRINT) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Generata binary file
bin_PROGRAMS = lingmo-powerserver lingmo-screensaverserver lingmo-sessionserver

#Add the dependent source file for powerserver
lingmo_powerserver_SOURCES = \
	power-generated.h power-generated.c \
	lingmo-power-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_powerserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_powerserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for screensaverserver
lingmo_screensaverserver_SOURCES = \
	screensaver-generated.h screensaver-generated.c \
	lingmo-screensaver-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_screensaverserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_screensaverserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)

#Add the dependent source file for sessionserver
lingmo_sessionserver_SOURCES = \
	session-generated.h session-generated.c \
	lingmo-session-server.c \
	$(NULL)

#Additional C compiler flags
lingmo_sessionserver_CFLAGS=  $(COMM_CFS) \
	$(GLIB_2_CFLAGS) $(GIO_2_CFLAGS) $(GIO_UNIX_2_CFLAGS) \
	$(NULL)

#Additional link objects
lingmo_sessionserver_LDFLAGS= $(COMM_PRINT) $(COMM_GSETTINGS) \
	$(GLIB_2_LIBS) $(GIO_2_LIBS) $(GIO_UNIX_2_LIBS) \
	$(NULL)
