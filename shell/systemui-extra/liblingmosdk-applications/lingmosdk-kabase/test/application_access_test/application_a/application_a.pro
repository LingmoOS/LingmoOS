TARGET = application_a

TEMPLATE = app

LIBS += -L../../../build/ -lkabase

HEADERS += fun.hpp

SOURCES += main.cpp \
          fun.cpp

include(../../../src/kabase.pri)