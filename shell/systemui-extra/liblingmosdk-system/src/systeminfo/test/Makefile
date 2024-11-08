CFLAGS += -g -O0 --std=c11 -I../
VPATH=../:./

all:
	gcc -o kysysinfo_test $(CFLAGS) kysysinfo_test.c ../libkysysinfo.c

clean:
	-rm kysysinfo_test