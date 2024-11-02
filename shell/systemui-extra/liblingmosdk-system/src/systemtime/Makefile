
FILE=$(file)
CFLAGS=$(cflags)
CFLAGS_DBUS = $(shell pkg-config --cflags --libs dbus-1)
CFLAGS_DBUS_GLIB = $(shell pkg-config --cflags --libs dbus-glib-1)
CFLAGS += -g -Wall -Werror -O0 -std=gnu11 -Wall


CLIBS=-lpthread -pthread -lrt 
CLIBS += $(clibs)

all: m_systime

m_systime: m_systime.c ./timer/libkytimer.c
	gcc m_systime.c ./timer/libkytimer.c -o $@ $(CFLAGS) $(CFLAGS_DBUS) $(CFLAGS_DBUS_GLIB) $(CLIBS) 


clean:
	rm -f m_systime

.PHONY: all clean
