TARGET = application_b

TEMPLATE = app

LIBS += -L../../../build/ -lkabase

HEADERS += fun.hpp

SOURCES += main.cpp \
          fun.cpp

include(../../../src/kabase.pri)