INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QMAKE_CXXFLAGS += -Wno-unused-parameter

QT -= gui
QT += xml gui-private
!build_xlsx_lib

SC_LIB_NAME = Docx
SC_LIB_DEPENDS += comutils

SOURCES += \
    $$PWD/document.cpp \
    $$PWD/shared.cpp \
    $$PWD/text.cpp \
    #blkcntnr.cpp \
    $$PWD/parts/documentpart.cpp \
    $$PWD/opc/part.cpp \
    $$PWD/opc/packuri.cpp \
    $$PWD/opc/opcpackage.cpp \
    $$PWD/package.cpp \
    $$PWD/opc/parts/corepropertiespart.cpp \
    $$PWD/opc/coreproperties.cpp \
    $$PWD/opc/oxml.cpp \
    $$PWD/opc/constants.cpp \
    $$PWD/oxml/parts/oxmldocument.cpp \
    $$PWD/oxml/oxmltext.cpp \
    $$PWD/table.cpp \
    $$PWD/oxml/xmlchemy.cpp \
    $$PWD/oxml/oxmltable.cpp \
    $$PWD/oxml/oxmlshape.cpp \
    $$PWD/shape.cpp \
    $$PWD/parts/imagepart.cpp \
    $$PWD/image/image.cpp \
    $$PWD/opc/physpkgreader.cpp \
    $$PWD/opc/packagereader.cpp \
    $$PWD/opc/contenttypemap.cpp \
    $$PWD/opc/serializedrelationships.cpp \
    $$PWD/opc/rel.cpp \
    $$PWD/opc/packagewriter.cpp \
    $$PWD/opc/physpkgwriter.cpp \
    $$PWD/enums/enumtext.cpp \
    $$PWD/length.cpp

HEADERS +=\
    $$PWD/docx_global.h \
    $$PWD/document.h \
    $$PWD/shared.h \
    $$PWD/text.h \
    #blkcntnr.h \
    $$PWD/parts/documentpart.h \
    $$PWD/opc/part.h \
    $$PWD/opc/packuri.h \
    $$PWD/opc/opcpackage.h \
    $$PWD/package.h \
    $$PWD/opc/parts/corepropertiespart.h \
    $$PWD/opc/coreproperties.h \
    $$PWD/opc/oxml.h \
    $$PWD/opc/constants.h \
    $$PWD/oxml/parts/oxmldocument.h \
    $$PWD/oxml/oxmltext.h \
    $$PWD/table.h \
    $$PWD/oxml/xmlchemy.h \
    $$PWD/oxml/oxmltable.h \
    $$PWD/oxml/oxmlshape.h \
    $$PWD/shape.h \
    $$PWD/parts/imagepart.h \
    $$PWD/image/image.h \
    $$PWD/opc/physpkgreader.h \
    $$PWD/opc/packagereader.h \
    $$PWD/opc/contenttypemap.h \
    $$PWD/opc/serializedrelationships.h \
    $$PWD/opc/rel.h \
    $$PWD/opc/packagewriter.h \
    $$PWD/opc/physpkgwriter.h \
    $$PWD/enums/enumtext.h \
    $$PWD/length.h


