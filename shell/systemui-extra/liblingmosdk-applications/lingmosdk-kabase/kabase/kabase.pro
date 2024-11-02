# 检测 Qt 版本
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION , ".")
QT_VER_MAJ = $$member(QT_VERSION , 0)
QT_VER_MIN = $$member(QT_VERSION , 1)
QT_VER_THR = $$member(QT_VERSION , 2)

QT += dbus core xml widgets svg network
TARGET = lingmosdk-kabase
TEMPLATE = lib

CONFIG += c++11 console link_pkgconfig no_keywords

LIBS += -ldl -lpthread -lsystemd -lkylog -lkyconf -lgif -L/usr/lib/lingmosdk/lingmosdk-base

greaterThan(QT_VER_MAJ , 4) {
    LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lstb -lfreeimage -lfreeimageplus
} else {
    message("--- v10 project ---")
}

PKGCONFIG += gsettings-qt openssl x11

INCLUDEPATH += /usr/include/lingmosdk/lingmosdk-base/ \
               /usr/include/opencv4/

HEADERS += buried_point.hpp \
           log.hpp \
           gsettings.hpp \
           currency.hpp \
           dbus.hpp \
           application_access.hpp \
           lingmo_system/xatom_helper.hpp \
           lingmo_system/window_management.hpp \
           lingmo_system/session_management.hpp \
           lingmo_system/user_manual.hpp \
           lingmo_system/system_information.hpp \
           lingmo_system/theme_management.hpp \
           single_application/single_application.hpp \
           single_application/locked_file.hpp \
           single_application/local_peer.hpp

greaterThan(QT_VER_MAJ , 4) {
    HEADERS += lingmo_image_codec/loadmovie.hpp \
               lingmo_image_codec/savemovie.hpp \
               lingmo_image_codec/lingmoimagecodec.hpp \
               lingmo_image_codec/lingmoimagecodec_global.hpp \
} else {
    message("+++ v10 project +++")
}


SOURCES += buried_point.cpp \
           log.cpp \
           gsettings.cpp \
           currency.cpp \
           dbus.cpp\
           application_access.cpp\
           lingmo_system/xatom_helper.cpp \
           lingmo_system/window_management.cpp \
           lingmo_system/session_management.cpp \
           lingmo_system/user_manual.cpp \
           lingmo_system/system_information.cpp \
           lingmo_system/theme_management.cpp \
           single_application/single_application.cpp \
           single_application/locked_file.cpp \
           single_application/local_peer.cpp \
           single_application/locked_file_unix.cpp

greaterThan(QT_VER_MAJ , 4) {
    SOURCES += lingmo_image_codec/image_conver/image_conver.cpp \
               lingmo_image_codec/image_load/image_load.cpp \
               lingmo_image_codec/image_save/image_save.cpp \
               lingmo_image_codec/image_load/loadmovie.cpp \
               lingmo_image_codec/image_save/savemovie.cpp \
               lingmo_image_codec/lingmoimagecodec.cpp \
} else {
    message("=== v10 project ===")
}

# 窗管模块
QT += x11extras KWindowSystem

# Default rules for deployment.
headers.files = application_access.hpp \
                buried_point.hpp \
                currency.hpp \
                dbus.hpp \
                gsettings.hpp \
                log.hpp
headers.path = /usr/include/lingmosdk/applications/kabase/

image_headers.files = lingmo_image_codec/lingmoimagecodec.hpp \
                lingmo_image_codec/lingmoimagecodec_global.hpp \
                lingmo_image_codec/loadmovie.hpp \
                lingmo_image_codec/savemovie.hpp
image_headers.path = /usr/include/lingmosdk/applications/kabase/lingmo_image_codec/

system_headers.files = lingmo_system/session_management.hpp \
                lingmo_system/system_information.hpp \
                lingmo_system/theme_management.hpp \
                lingmo_system/user_manual.hpp \
                lingmo_system/window_management.hpp
system_headers.path = /usr/include/lingmosdk/applications/kabase/lingmo_system/

single_app_headers.files = single_application/single_application.hpp
single_app_headers.path = /usr/include/lingmosdk/applications/kabase/single_application/

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers image_headers system_headers single_app_headers
