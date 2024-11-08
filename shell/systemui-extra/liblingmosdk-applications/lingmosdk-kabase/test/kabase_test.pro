QT += core widgets dbus svg

TARGET = kabase-test
TEMPLATE = app
QMAKE_CXXFLAGS += -g -Wall

DEFINES += QT_MESSAGELOGCONTEXT

CONFIG += c++11
INCLUDEPATH += ../kabase \
               ../kabase/single_application \
               ../kabase/lingmo_system \
               ../kabase/lingmo_image_codec \
               /usr/include/opencv4/

LIBS += -L../build-so -llingmosdk-kabase -Wl,-rpath=/home/snow/snow/code/lingmosdk-application/lingmosdk-kabase/build-so -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

HEADERS += widget_test.hpp \
           dbus_test.hpp\
           gsettings_test.hpp \
           image_test.hpp

SOURCES += main.cpp \
           widget_test.cpp \
           dbus_test.cpp \
           gsettings_test.cpp \
           image_test.cpp