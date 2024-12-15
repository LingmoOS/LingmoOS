TARGET  = xraw
TEMPLATE = lib
CONFIG += \
    c++11 \
    link_pkgconfig \
    plugin
DESTDIR = imageformats

PKGCONFIG += \
    libraw

HEADERS += \
    datastream.h \
    rawiohandler.h
SOURCES += \
    datastream.cpp \
    main.cpp \
    rawiohandler.cpp
OTHER_FILES += \
    raw.json

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target

# For KDE, install a .desktop file with metadata about the loader
kde_desktop.files = raw.desktop
kde_desktop.path = $${INSTALL_KDEDIR}/share/kde4/services/qimageioplugins/
#INSTALLS += kde_desktop
