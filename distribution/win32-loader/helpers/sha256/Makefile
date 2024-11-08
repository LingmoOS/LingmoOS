# Licensed under the zlib/libpng license (same as NSIS)

CC = i686-w64-mingw32-gcc
CFLAGS = -Os -Wall -pedantic -s
LDFLAGS = -s -shared -Wl,--add-stdcall-alias
LIBS = -nostdlib -lkernel32

TARGET = sha256.dll
sha256_OBJS = sha256.o sha256-x86.o

.PHONY: clean

all:: $(TARGET)

clean::
	rm -f $(TARGET) $(sha256_OBJS)

$(TARGET): $(sha256_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
