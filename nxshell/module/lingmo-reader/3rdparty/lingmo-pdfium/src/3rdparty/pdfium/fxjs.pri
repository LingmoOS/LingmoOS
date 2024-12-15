# module fxjs
SOURCE_DIR = $$PWD/pdfium

HEADERS += \
    $$PWD/pdfium/fxjs/cjs_event_context_stub.h \
    $$PWD/pdfium/fxjs/cjs_runtimestub.h \
    $$PWD/pdfium/fxjs/cjs_event_context.h \
    $$PWD/pdfium/fxjs/ijs_runtime.h

SOURCES += \
    $$PWD/pdfium/fxjs/cjs_event_context_stub.cpp \
    $$PWD/pdfium/fxjs/cjs_runtimestub.cpp \
    $$PWD/pdfium/fxjs/ijs_runtime.cpp

