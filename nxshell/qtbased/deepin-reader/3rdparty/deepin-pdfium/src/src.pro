TARGET = $$PWD/../lib/deepin-pdfium

TEMPLATE = lib

CONFIG += c++14 link_pkgconfig

PKGCONFIG += chardet

isEmpty(PREFIX) {
 PREFIX = /usr
}
DEFINES += QMAKE_INSTALL_PREFIX=\"\\\"$$PREFIX\\\"\"

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

QT += core-private core gui

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

include($$PWD/3rdparty/pdfium/pdfium.pri)

INCLUDEPATH += $$PWD/../include

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

target.path  = $$PREFIX/lib

INSTALLS += target
