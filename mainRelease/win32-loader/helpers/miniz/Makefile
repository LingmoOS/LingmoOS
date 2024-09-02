# Licensed under the zlib/libpng license (same as NSIS)

CC = i686-w64-mingw32-gcc
CPPFLAGS = -DMINIZ_NO_ARCHIVE_APIS=1 -DMINIZ_NO_MALLOC=1 \
	-DMINIZ_NO_ZLIB_APIS=1 -DNDEBUG
CFLAGS = -Os -ffunction-sections -pedantic
LDFLAGS = -s -shared -Wl,--add-stdcall-alias,--gc-sections
LIBS = -nostdlib -static-libgcc -lgcc -lkernel32

TARGET = miniz.dll
OBJS = main.o miniz.o miniz_tdef.o

.PHONY:	clean

all::	$(TARGET)

clean::
	rm -f $(TARGET) $(OBJS)

$(TARGET):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
