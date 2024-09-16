SOURCE_DIR = $$PWD/pdfium

INCLUDEPATH += $$SOURCE_DIR

#OPJ_STATIC
#FT2_BUILD_LIBRARY

DEFINES += __QT__ \
    PNG_PREFIX \
    PNG_USE_READ_MACROS \
    BUILD_DEEPIN_PDFIUM_LIB \
    DEFINE_PS_TABLES_DATA \
    FT_CONFIG_OPTION_ADOBE_GLYPH_LIST

DEFINES +=  USE_SYSTEM_LIBJPEG \
            USE_SYSTEM_ZLIB \
            USE_SYSTEM_LIBPNG \
            USE_SYSTEM_ICUUC \
            USE_SYSTEM_LCMS2 \
            USE_SYSTEM_LIBOPENJPEG2 \
            USE_SYSTEM_FREETYPE

CONFIG += link_pkgconfig
PKGCONFIG += libopenjp2 lcms2 freetype2

#QMAKE_CXXFLAGS += "-Wc++11-narrowing"  #is_clang
#QMAKE_CXXFLAGS += "-Wno-inconsistent-missing-override"  #is_clang Suppress no override warning for overridden functions.
#QMAKE_CXXFLAGS += "-Wno-switch" #is_clang http://code.google.com/p/pdfium/issues/detail?id=188

contains(QMAKE_HOST.arch, x86_64)  {
    DEFINES += "_FX_CPU_=_FX_X64_"
    QMAKE_CXXFLAGS += "-fPIC"
    DEFINES += ARCH_CPU_ARM64
}

contains(QMAKE_HOST.arch, mips64):{
    DEFINES += _MIPS_ARCH_LOONGSON  #mips暂时默认为龙芯 调整最小页尺寸
    QMAKE_CXXFLAGS += "-O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi"
}

contains(QMAKE_HOST.arch, sw64):{
    DEFINES += ARCH_CPU_SW64
    QMAKE_CXXFLAGS += "-fPIC"
}

include(fx_freetype.pri)
include(fpdfsdk.pri)
include(core.pri)
#include(fx_libopenjpeg.pri)
include(fx_agg.pri)
include(fxjs.pri)
#include(fx_lcms2.pri)
include(fx_skia.pri)
include(fx_base.pri)

HEADERS += \
    $$PWD/pdfium/public/cpp/fpdf_deleters.h \
    $$PWD/pdfium/public/cpp/fpdf_scopers.h \
    $$PWD/pdfium/public/fpdf_annot.h \
    $$PWD/pdfium/public/fpdf_attachment.h \
    $$PWD/pdfium/public/fpdf_catalog.h \
    $$PWD/pdfium/public/fpdf_dataavail.h \
    $$PWD/pdfium/public/fpdf_doc.h \
    $$PWD/pdfium/public/fpdf_edit.h \
    $$PWD/pdfium/public/fpdf_ext.h \
    $$PWD/pdfium/public/fpdf_flatten.h \
    $$PWD/pdfium/public/fpdf_formfill.h \
    $$PWD/pdfium/public/fpdf_fwlevent.h \
    $$PWD/pdfium/public/fpdf_javascript.h \
    $$PWD/pdfium/public/fpdf_ppo.h \
    $$PWD/pdfium/public/fpdf_progressive.h \
    $$PWD/pdfium/public/fpdf_save.h \
    $$PWD/pdfium/public/fpdf_searchex.h \
    $$PWD/pdfium/public/fpdf_signature.h \
    $$PWD/pdfium/public/fpdf_structtree.h \
    $$PWD/pdfium/public/fpdf_sysfontinfo.h \
    $$PWD/pdfium/public/fpdf_text.h \
    $$PWD/pdfium/public/fpdf_thumbnail.h \
    $$PWD/pdfium/public/fpdf_transformpage.h \
    $$PWD/pdfium/public/fpdfview.h
