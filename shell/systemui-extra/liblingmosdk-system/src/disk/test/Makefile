LDFLAGS =
FILE=$(file)
CFLAGS=$(cflags)
CFLAGS += -g -O0 -std=gnu11 -Wall -DPWD=\"$(PWD)\" -I/usr/include/glib-2.0/glib/ -I../ -I../../../lingmosdk-base/src/utils -I../../../lingmosdk-base/src/log/
CLIBS= -lblkid -lmount -L/home/lingmo/git-local-storage/lingmosdk/lingmosdk-base/src/log/lib -Wl,-rpath=/home/lingmo/git-local-storage/lingmosdk/lingmosdk-base/src/log/lib -lkylog -L/home/lingmo/git-local-storage/lingmosdk/lingmosdk-base/src/config/lib -Wl,-rpath=/home/lingmo/git-local-storage/lingmosdk/lingmosdk-base/src/config/lib -lkyconf
CLIBS += $(clibs)

all:getdiskinfo getdisklist

getdisklist:getdisklist.c ../diskinfo.c
	gcc -o $(basename $<) $< ../diskinfo.c $(CFLAGS) $(CLIBS)

getdiskinfo:getdiskinfo.c ../diskinfo.c
	gcc -o $(basename $<) $< ../diskinfo.c $(CFLAGS) $(CLIBS)
