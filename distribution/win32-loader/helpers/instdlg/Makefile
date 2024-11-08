# Licensed under the zlib/libpng license (same as NSIS)

AS = i686-w64-mingw32-as
LD = i686-w64-mingw32-ld
IMAGE_BASE = 0x400000
ASFLAGS = --defsym IMAGE_BASE=$(IMAGE_BASE) -march=i486
LDFLAGS = --image-base $(IMAGE_BASE) --strip-all --subsystem windows

TARGET = instdlg.exe
instdlg_OBJS = instdlg.o

.PHONY: clean

all:: $(TARGET)

clean::
	rm -f $(TARGET) $(instdlg_OBJS)

$(TARGET): $(instdlg_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
