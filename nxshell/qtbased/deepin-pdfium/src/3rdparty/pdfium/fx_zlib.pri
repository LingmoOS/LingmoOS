# module fx_zlib
SOURCE_DIR = $$PWD/pdfium/third_party

QMAKE_CXXFLAGS += "-Wno-parentheses-equality"

INCLUDEPATH += $$PWD/pdfium/third_party/zlib
INCLUDEPATH += $$PWD/pdfium/third_party/zlib/contrib/optimizations
INCLUDEPATH += $$PWD/pdfium/third_party/zlib/contrib/minizip

HEADERS += \
    $$PWD/pdfium/third_party/zlib/contrib/optimizations/insert_string.h \
    $$PWD/pdfium/third_party/zlib/chromeconf.h \
    $$PWD/pdfium/third_party/zlib/cpu_features.h \
    $$PWD/pdfium/third_party/zlib/crc32.h \
    $$PWD/pdfium/third_party/zlib/crc32_simd.h \
    $$PWD/pdfium/third_party/zlib/deflate.h \
    $$PWD/pdfium/third_party/zlib/gzguts.h \
    $$PWD/pdfium/third_party/zlib/inffast.h \
    $$PWD/pdfium/third_party/zlib/inffixed.h \
    $$PWD/pdfium/third_party/zlib/inflate.h \
    $$PWD/pdfium/third_party/zlib/inftrees.h \
    $$PWD/pdfium/third_party/zlib/trees.h \
    $$PWD/pdfium/third_party/zlib/zconf.h \
    $$PWD/pdfium/third_party/zlib/zlib.h \
    $$PWD/pdfium/third_party/zlib/zutil.h \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/ioapi.h \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/unzip.h \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/zip.h

#    $$PWD/pdfium/third_party/zlib/contrib/optimizations/chunkcopy.h \
#    $$PWD/pdfium/third_party/zlib/contrib/optimizations/inffast_chunk.h \
#    $$PWD/pdfium/third_party/zlib/contrib/optimizations/slide_hash_neon.h \
#    $$PWD/pdfium/third_party/zlib/contrib/tests/infcover.h \
#    $$PWD/pdfium/third_party/zlib/google/compression_utils.h \
#    $$PWD/pdfium/third_party/zlib/google/compression_utils_portable.h \
#    $$PWD/pdfium/third_party/zlib/google/zip.h \
#    $$PWD/pdfium/third_party/zlib/google/zip_internal.h \
#    $$PWD/pdfium/third_party/zlib/google/zip_reader.h \
#    $$PWD/pdfium/third_party/zlib/google/zip_writer.h \
#    $$PWD/pdfium/third_party/zlib/adler32_simd.h \

SOURCES += \
    $$PWD/pdfium/third_party/zlib/adler32.c \
    $$PWD/pdfium/third_party/zlib/compress.c \
    $$PWD/pdfium/third_party/zlib/cpu_features.c \
    $$PWD/pdfium/third_party/zlib/crc32.c \
    $$PWD/pdfium/third_party/zlib/crc32_simd.c \
    $$PWD/pdfium/third_party/zlib/deflate.c \
    $$PWD/pdfium/third_party/zlib/gzclose.c \
    $$PWD/pdfium/third_party/zlib/gzlib.c \
    $$PWD/pdfium/third_party/zlib/gzread.c \
    $$PWD/pdfium/third_party/zlib/gzwrite.c \
    $$PWD/pdfium/third_party/zlib/infback.c \
    $$PWD/pdfium/third_party/zlib/inffast.c \
    $$PWD/pdfium/third_party/zlib/inftrees.c \
    $$PWD/pdfium/third_party/zlib/trees.c \
    $$PWD/pdfium/third_party/zlib/uncompr.c \
    $$PWD/pdfium/third_party/zlib/zutil.c \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/ioapi.c \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/unzip.c \
    $$PWD/pdfium/third_party/zlib/contrib/minizip/zip.c

#    $$PWD/pdfium/third_party/zlib/contrib/minizip/unzip.c \
#    $$PWD/pdfium/third_party/zlib/contrib/minizip/zip.c \
#    $$PWD/pdfium/third_party/zlib/contrib/optimizations/inffast_chunk.c \
#    $$PWD/pdfium/third_party/zlib/contrib/optimizations/inflate.c \
#    $$PWD/pdfium/third_party/zlib/crc_folding.c \
#    $$PWD/pdfium/third_party/zlib/fill_window_sse.c \
#    $$PWD/pdfium/third_party/zlib/inflate.c \
#    $$PWD/pdfium/third_party/zlib/adler32_simd.c \

