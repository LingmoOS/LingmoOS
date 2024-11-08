LDFLAGS =
FILE=$(file)
CFLAGS=$(cflags)
CFLAGS += -g -O0 -std=gnu11 -Wall -DPWD=\"$(PWD)\" -I/usr/include/glib-2.0/glib/ -I../ -I../../utils/
CLIBS= -lpthread
CLIBS += $(clibs)

all:list

list:test_structlist.c ../configure.c ../structparse.c ../jsonparse.c ../gsettingsparse.c ../xmlparse.c
	gcc -o $(basename $<) $^ $(CFLAGS) $(CLIBS)
