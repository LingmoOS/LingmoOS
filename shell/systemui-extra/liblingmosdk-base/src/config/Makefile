OBJ=gsettingsparse.o jsonparse.o structparse.o xmlparse.o

CFLAGS=-g -O0 -Wall -fPIC -I../ -I../utils/ -I../config/
CLIBS=-lpthread
CC:=$(shell bash -c 'type $(CC) &> /dev/null && echo $(CC) || echo gcc')
TARGET=libkyconf.so
LIBINST=libkyconf.so.1.0.0
LIBHEAD=kyconf.h
LIBINSTHEADER=/usr/include/lingmosdk/base/

all:$(OBJ)
	$(CC) -o $(LIBINST) configure.c $(OBJ) $(CFLAGS) $(CLIBS) -shared
	mkdir -p lib/
	mv -f $(LIBINST) lib/
	cd lib/ && ln -sf $(LIBINST) $(TARGET) && cd ..
	-rm *.o &> /dev/null

clean:
	-rm *.o &> /dev/null
	-rm ../lib/$(TARGET) &> /dev/null