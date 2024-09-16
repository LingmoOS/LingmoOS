TARGET = $$PWD/../lib/deepin-pdfium

TEMPLATE = lib

CONFIG += c++14 link_pkgconfig

###安全漏洞检测
#QMAKE_CXX += -g -fsanitize=undefined,address -O2
#QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
#QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2

#安全编译参数
QMAKE_CFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy

#获取当前系统页大小
PageSize = $$system(getconf PAGESIZE)
DEFINES += "SYSTEMPAGESIZE=$$PageSize"
message("DEFINES: "$$DEFINES)

DEFINES += BUILD_DEEPDF_LIB

QT += core gui

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

include($$PWD/3rdparty/pdfium/pdfium.pri)

INCLUDEPATH += $$PWD/../include

PKGCONFIG += chardet lcms2 freetype2 libopenjp2

LIBS += -lz -ljpeg -licuuc

public_headers += \
    $$PWD/../include/dpdfglobal.h \
    $$PWD/../include/dpdfdoc.h \
    $$PWD/../include/dpdfpage.h \
    $$PWD/../include/dpdfannot.h

HEADERS += $$public_headers

SOURCES += \
    $$PWD/dpdfglobal.cpp \
    $$PWD/dpdfdoc.cpp \
    $$PWD/dpdfpage.cpp \
    $$PWD/dpdfannot.cpp

isEmpty(PREFIX): PREFIX = /usr

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$PREFIX/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}
isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/deepin-pdfium
} else {
    includes.path = $$INCLUDE_INSTALL_DIR
}

includes.files += $$PWD/../include/*.h

INSTALLS += includes target


#----------This is the configuration for generating the pkg-config file
CONFIG += create_pc create_prl no_install_prl

# This is our libdir
QMAKE_PKGCONFIG_LIBDIR = $$target.path

# Usually people take the semver version here
QMAKE_PKGCONFIG_VERSION = $$VERSION

# This is where our API specific headers are
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_INCDIR = $$includes.path

# This fills in the Name property
QMAKE_PKGCONFIG_NAME = deepin-pdfium

# This fills in the Description property
QMAKE_PKGCONFIG_DESCRIPTION = deepin-pdfium Library with Qt5

