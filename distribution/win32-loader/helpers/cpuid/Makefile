# Licensed under the zlib/libpng license (same as NSIS)

AS = i686-w64-mingw32-as
LD = i686-w64-mingw32-ld
ASFLAGS = -march=i486
LDFLAGS = -m i386pe --add-stdcall-alias --dll --dynamicbase \
	--enable-auto-image-base --entry _DllEntryPoint@12 \
	--nxcompat --strip-all --subsystem windows

TARGET = cpuid.dll
cpuid_OBJS = cpuid_dll.o

.PHONY: clean

all:: $(TARGET)

clean::
	rm -f $(TARGET) $(cpuid_OBJS)

cpuid.dll: $(cpuid_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
