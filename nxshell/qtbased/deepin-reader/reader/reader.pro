#config
QT += core gui sql printsupport dbus widgets

PKGCONFIG += ddjvuapi dtkwidget

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

QMAKE_CXXFLAGS += -fPIE

QMAKE_LFLAGS += -pie

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
message("APP_VERSION: "$$VERSION)

contains(QMAKE_HOST.arch, mips64):{
    QMAKE_CXXFLAGS += "-O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi"
}

#代码覆盖率开关
if(contains(DEFINES, CMAKE_COVERAGE_ARG_ON)){
    QMAKE_CFLAGS += -g -Wall -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage
}

CONFIG += c++11 link_pkgconfig

TARGET = deepin-reader

TEMPLATE = app

#DEFINES += PERF_ON

# 判断系统环境
MAJOR_VERSION=$$system("cat /etc/os-version | grep MajorVersion | grep -o '[0-9]\+'")
#message("MAJOR_VERSION: " $$MAJOR_VERSION)
equals(MAJOR_VERSION, 23) {
    message("----------------- OS_BUILD_V23 on")
    DEFINES += OS_BUILD_V23
}

#Install
target.path   = $$PREFIX/bin

desktop.path  = $$PREFIX/share/applications

desktop.files = $$PWD/deepin-reader.desktop

icon.path = $$PREFIX/share/icons/hicolor/scalable/apps

icon.files = $$PWD/deepin-reader.svg

manual.path = $$PREFIX/share/deepin-manual/manual-assets/application

manual.files = $$PWD/../assets/*

INSTALLS += target desktop icon manual

#translate
TRANSLATIONS += $$PWD/../translations/deepin-reader.ts

CONFIG(release, debug|release) {
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    TRANSLATIONFILES= $$files($$PWD/../translations/*.ts)
    for(tsfile, TRANSLATIONFILES) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = $$PREFIX/share/$$TARGET/translations
    dtk_translations.files = $$PWD/../translations/*.qm
    INSTALLS += dtk_translations
}

#code
SOURCES += \
    $$PWD/main.cpp \

include($$PWD/src.pri)
