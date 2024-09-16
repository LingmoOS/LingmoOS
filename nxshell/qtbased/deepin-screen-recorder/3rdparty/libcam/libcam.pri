HEADERS += \
    $$PWD/libcam_v4l2core/*.h \
    $$PWD/libcam_encoder/*.h \
    $$PWD/libcam_render/*.h \
    $$PWD/libcam_audio/*.h \
    $$PWD/libcam/*.h \
    $$PWD/load_libs.h

SOURCES += \
    $$PWD/libcam_v4l2core/*.c \
    $$PWD/libcam_encoder/*.c \
    $$PWD/libcam_render/*.c \
    $$PWD/libcam_audio/*.c \
    $$PWD/libcam/*.c \
    $$PWD/load_libs.c

INCLUDEPATH += $$PWD
