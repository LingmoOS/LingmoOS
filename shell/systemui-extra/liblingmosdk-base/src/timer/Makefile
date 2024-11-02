CLIBS=-lpthread -lrt
CFLAGS=-g -O0 -Wall -fPIC -shared

CC=cc
TARGET=libkytimer.so
LIBINSTALLNAME=libkytimer.so.1.0.0
LIBINSTALLDIR=/usr/lib/
HEADERINSTALLDIR=/usr/include/
HEADERS=kytimer.h

.PHONY:all
.PHONY:clean
.PHONY:lib

all:lib

lib:
	$(CC) $(CFLAGS) $(CLIBS) -o $(TARGET) libkytimer.c
	mkdir -p lib/
	mv $(TARGET) lib/$(LIBINSTALLNAME)

clean:
	-rm ../lib/$(LIBINSTALLNAME)

target_install:TARGET
	cp $(LIBINSTALLNAME) $(LIBINSTALLDIR)
