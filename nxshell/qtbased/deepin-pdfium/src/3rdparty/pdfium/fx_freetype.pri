# module fx_freetype
SOURCE_DIR = $$PWD/pdfium/third_party

INCLUDEPATH += $$PWD/pdfium/third_party/freetype/include \
               $$PWD/pdfium/third_party/freetype/include/freetype

HEADERS += \
      $$PWD/pdfium/third_party/freetype/include/pstables.h \
      $$PWD/pdfium/third_party/freetype/src/src/cff/cffobjs.h \

HEADERS += \
    $$PWD/pdfium/third_party/freetype/include/freetype-custom-config/ftmodule.h \
    $$PWD/pdfium/third_party/freetype/include/freetype-custom-config/ftoption.h

DISTFILES += \
    $$PWD/pdfium/third_party/freetype/src/src/autofit/afblue.dat
