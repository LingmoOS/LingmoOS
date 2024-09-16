#config
TEMPLATE = app

TARGET = htmltopdf

QT += webenginewidgets

isEmpty(PREFIX) {
 PREFIX = /usr
}
DEFINES += QMAKE_INSTALL_PREFIX=\"\\\"$$PREFIX\\\"\"

target.path = $$PREFIX/lib/deepin-reader

INSTALLS += target

###安全漏洞检测
#QMAKE_CXX += -g -fsanitize=undefined,address -O2
#QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
#QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2

#安全编译参数
QMAKE_CFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy

QMAKE_CXXFLAGS += -fPIE

QMAKE_LFLAGS += -pie

contains(QMAKE_HOST.arch, mips64):{
    QMAKE_CXXFLAGS += "-O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi"
}

CONFIG += c++11

#code
HEADERS += \
    htmltopdfconverter.h

SOURCES += \
    main.cpp \
    htmltopdfconverter.cpp
