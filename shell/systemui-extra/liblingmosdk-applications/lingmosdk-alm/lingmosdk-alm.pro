QT += core network widgets KWindowSystem

TARGET = lingmosdk-alm
TEMPLATE = lib
CONFIG += c++11 console

HEADERS += src/singleapplication/singleapplication.h \
           src/singleapplication/localpeer.h \

SOURCES += src/singleapplication/singleapplication.cpp \
           src/singleapplication/localpeer.cpp \

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/lingmosdk/applications/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers
