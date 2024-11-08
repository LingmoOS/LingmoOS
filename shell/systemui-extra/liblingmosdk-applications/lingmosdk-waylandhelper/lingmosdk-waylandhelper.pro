QT += KWaylandClient KWaylandServer gui widgets x11extras KWindowSystem KIconThemes concurrent

TEMPLATE = lib
DEFINES += LINGMOSDKWAYLANDHELPER_LIBRARY

CONFIG += c++11 link_pkgconfig
PKGCONFIG += wayland-client x11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_LINGMO_PROTOCOL

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

system(wayland-scanner client-header /usr/share/lingmo-wayland-protocols/lingmo-shell.xml src/windowmanager/lingmo-shell-client-protocol.h)
system(wayland-scanner private-code /usr/share/lingmo-wayland-protocols/lingmo-shell.xml src/windowmanager/lingmo-shell-core-protocol.c)

system(wayland-scanner client-header /usr/share/lingmo-wayland-protocols/lingmo-window-management.xml src/windowmanager/lingmo-window-management-client-protocol.h)
system(wayland-scanner private-code /usr/share/lingmo-wayland-protocols/lingmo-window-management.xml src/windowmanager/lingmo-window-management-core-protocol.c)

system(wayland-scanner client-header /usr/share/wayland-protocols/staging/xdg-activation/xdg-activation-v1.xml src/windowmanager/wayland-xdg-activation-v1-client-protocol.h)
system(wayland-scanner private-code /usr/share/wayland-protocols/staging/xdg-activation/xdg-activation-v1.xml src/windowmanager/wayland-xdg-activation-v1-protocol.c)

system(wayland-scanner client-header /usr/share/wayland-protocols/staging/ext-idle-notify/ext-idle-notify-v1.xml src/windowmanager/wayland-ext-idle-notify-v1-client-protocol.h)
system(wayland-scanner private-code /usr/share/wayland-protocols/staging/ext-idle-notify/ext-idle-notify-v1.xml src/windowmanager/wayland-ext-idle-notify-v1-protocol.c)

system(wayland-scanner client-header /usr/share/lingmo-wayland-protocols/lingmo-blur.xml src/blurmanager/lingmo-blur-manager-client-protocol.h)
system(wayland-scanner private-code /usr/share/lingmo-wayland-protocols/lingmo-blur.xml src/blurmanager/lingmo-blur-manager-core-protocol.c)

SOURCES += \
    src/lingmostylehelper/lingmo-decoration-core.c \
    src/lingmostylehelper/lingmo-decoration-manager.cpp \
    src/lingmostylehelper/lingmostylehelper.cpp \
    src/lingmostylehelper/xatom-helper.cpp \
    src/lingmostylehelper/lingmo-shell-manager.cpp \
    src/waylandhelper.cpp \
    src/windowmanager/abstractinterface.cpp \
    src/windowmanager/kdewaylandinterface.cpp \
    src/windowmanager/outputinfo.cpp \
    src/windowmanager/lingmowaylandinterface.cpp \
    src/windowmanager/xcbinterface.cpp \
    src/windowmanager/lingmowindowmanagement.cpp \
    src/windowmanager/wmregister.cpp \
    src/windowmanager/windowmanager.cpp \
    src/windowmanager/extidlenotifier.cpp \
    src/windowmanager/idlenotifier.cpp \
    src/windowmanager/lingmoshell.cpp \
    src/windowmanager/xdgactivation.cpp \
    src/windowmanager/wayland-plasma-window-management-protocol.c \
    src/windowmanager/lingmo-window-management-core-protocol.c\
    src/windowmanager/lingmo-shell-core-protocol.c \
    src/windowmanager/wayland-xdg-activation-v1-protocol.c \
    src/windowmanager/wayland-ext-idle-notify-v1-protocol.c \
    src/blurmanager/lingmoblur.cpp \
    src/blurmanager/blurmanager.cpp \
    src/blurmanager/lingmo-blur-manager-core-protocol.c \
    src/blurmanager/blurwaylandinterface.cpp \
    src/blurmanager/blurxcbinterface.cpp \


HEADERS += \
    src/lingmostylehelper/lingmo-decoration-client.h \
    src/lingmostylehelper/lingmo-decoration-manager.h \
    src/lingmostylehelper/lingmostylehelper.h \
    src/lingmostylehelper/xatom-helper.h \
    src/lingmostylehelper/lingmo-shell-manager.h \
    src/waylandhelper.h \
    src/windowmanager/abstractinterface.h \
    src/windowmanager/kdewaylandinterface.h \
    src/windowmanager/outputinfo.h \
    src/windowmanager/xcbinterface.h \
    src/windowmanager/lingmowaylandinterface.h \
    src/windowmanager/lingmowaylandpointer.h \
    src/windowmanager/lingmowindowmanagement.h \
    src/windowmanager/windowinfo.h \
    src/windowmanager/wmregister.h \
    src/windowmanager/windowmanager.h \
    src/windowmanager/xdgactivation.h \
    src/windowmanager/lingmoshell.h \
    src/windowmanager/extidlenotifier.h \
    src/windowmanager/idlenotifier.h \
    src/windowmanager/lingmo-shell-client-protocol.h \
    src/windowmanager/wayland-plasma-window-management-client-protocol.h \
    src/windowmanager/lingmo-window-management-client-protocol.h \
    src/windowmanager/wayland-xdg-activation-v1-client-protocol.h \
    src/windowmanager/wayland-ext-idle-notify-v1-client-protocol.h \
    src/windowmanager/kyatoms_p.h \
    src/blurmanager/lingmoblur.h \
    src/blurmanager/blurmanager.h \
    src/blurmanager/lingmo-blur-manager-client-protocol.h\
    src/blurmanager/blurabstractinterface.h \
    src/blurmanager/blurwaylandinterface.h \
    src/blurmanager/blurxcbinterface.h \

# Default rules for deployment.
headers.files = src/lingmosdk-waylandhelper_global.h \
                src/waylandhelper.h
headers.path = /usr/include/lingmosdk/applications/

wm_headers.files = src/windowmanager/abstractinterface.h \
    src/windowmanager/kdewaylandinterface.h \
    src/windowmanager/xcbinterface.h \
    src/windowmanager/lingmowaylandinterface.h \
    src/windowmanager/lingmowaylandpointer.h \
    src/windowmanager/lingmowindowmanagement.h \
    src/windowmanager/windowinfo.h \
    src/windowmanager/wmregister.h \
    src/windowmanager/windowmanager.h \
    src/windowmanager/xdgactivation.h \
    src/windowmanager/lingmoshell.h \
    src/windowmanager/extidlenotifier.h \
    src/windowmanager/idlenotifier.h \
    src/windowmanager/lingmo-shell-client-protocol.h \
    src/windowmanager/wayland-plasma-window-management-client-protocol.h \
    src/windowmanager/lingmo-window-management-client-protocol.h \
    src/windowmanager/wayland-xdg-activation-v1-client-protocol.h \
    src/windowmanager/wayland-ext-idle-notify-v1-client-protocol.h \
    src/windowmanager/kyatoms_p.h \
    src/windowmanager/outputinfo.h \
                    
wm_headers.path = /usr/include/lingmosdk/applications/windowmanager/

sty_headers.files = src/lingmostylehelper/lingmo-decoration-client.h \
                    src/lingmostylehelper/lingmo-decoration-manager.h \
                    src/lingmostylehelper/lingmostylehelper.h \
                    src/lingmostylehelper/xatom-helper.h \
                    src/lingmostylehelper/lingmo-shell-manager.h


sty_headers.path = /usr/include/lingmosdk/applications/lingmostylehelper/

blur_headers.files = src/blurmanager/lingmoblur.h \
                    src/blurmanager/blurmanager.h \
                    src/blurmanager/lingmo-blur-manager-client-protocol.h\
                    src/blurmanager/blurabstractinterface.h \
                    src/blurmanager/blurwaylandinterface.h \
                    src/blurmanager/blurxcbinterface.h

blur_headers.path = /usr/include/lingmosdk/applications/blurmanager/

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers wm_headers sty_headers blur_headers
