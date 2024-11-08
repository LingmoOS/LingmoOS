OBJ=core.o format.o klog_mqueue.o kylog.o write.o writeFile.o

CFLAGS=-g -O0 -Wall -fPIC -I../ -I../utils/ -I../config/
# CLIBS=-lpthread -lsystemd 
CC:=$(shell bash -c 'type $(CC) &> /dev/null && echo $(CC) || echo gcc')
TARGET=libkylog.so
LIBINST=libkylog.so.1.0.0
LIBHEAD=kylog.h
LIBINSTHEADER=/usr/include/kylog/

all:$(OBJ)
	$(CC) -o $(LIBINST) $(OBJ) $(CFLAGS) $(CLIBS) -shared
	mkdir -p lib/
	mv -f $(LIBINST) lib/
	cd lib/ && ln -sf $(LIBINST) $(TARGET) && cd ..
	-rm *.o &> /dev/null

clean:
	-rm *.o &> /dev/null
	-rm ../lib/$(TARGET) &> /dev/null