TARGET = docparser

TEMPLATE = lib

CONFIG += c++11 link_pkgconfig
PKGCONFIG += poppler-cpp libzip pugixml freetype2 libxml-2.0 uuid tinyxml2

#安全编译参数
QMAKE_CFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

###安全漏洞检测
#QMAKE_CXX += -g -fsanitize=undefined,address -O2
#QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
#QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2

include($$PWD/../3rdparty/3rdparty.pri)

HEADERS += \
    docparser.h \
    ofd/ofd.h

SOURCES += \
    docparser.cpp \
    ofd/ofd.cpp

INCLUDEPATH += $$PWD/../3rdparty/libs

isEmpty(PREFIX): PREFIX = /usr

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$PREFIX/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}
isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/docparser
} else {
    includes.path = $$INCLUDE_INSTALL_DIR
}

includes.files += $$PWD/*.h

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
QMAKE_PKGCONFIG_NAME = docparser

# This fills in the Description property
QMAKE_PKGCONFIG_DESCRIPTION = document parse library
