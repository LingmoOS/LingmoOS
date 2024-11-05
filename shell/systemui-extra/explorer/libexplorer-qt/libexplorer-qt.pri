include(explorer-core.pri)
include(file-operation/file-operation.pri)
include(file-launcher/file-launcher.pri)
include(model/model.pri)
#search vfs extension based on explorer-qt core.
include(vfs/vfs.pri)
#plugin interface
include(../plugin-iface/plugin-iface.pri)
include(../plugin-iface/unstable/window-plugin-iface.pri)
include(effects/effects.pri)

include(convenient-utils/convenient-utils.pri)

RESOURCES += $$PWD/libexplorer-qt.qrc

#ui-controls
include(controls/controls.pri)

#windows
include(windows/windows.pri)

# preview
include(thumbnail/thumbnail.pri)

# kyudfburn
contains(DEFINES, "LINGMO_UDF_BURN") {
    PKGCONFIG += kyudfburn
}

# lingmo-search
contains(DEFINES, "LINGMO_LINGMO_SEARCH") {
    PKGCONFIG += lingmo-search
}


HEADERS += \
    $$PWD/extensions-manager-widget.h \
    $$PWD/properties-window-factory-plugin-manager.h \
    $$PWD/file-copy.h               \
    $$PWD/explorer-log.h               \
    $$PWD/plugin-manager.h          \
    $$PWD/global-settings.h         \
    $$PWD/complementary-style.h     \
    $$PWD/global-fstabdata.h        \
    $$PWD/sound-effect.h \
    $$PWD/trash-cleaned-watcher.h

SOURCES += \
    $$PWD/extensions-manager-widget.cpp \
    $$PWD/properties-window-factory-plugin-manager.cpp \
    $$PWD/file-copy.cpp             \
    $$PWD/plugin-manager.cpp        \
    $$PWD/global-settings.cpp       \
    $$PWD/complementary-style.cpp   \
    $$PWD/global-fstabdata.cpp      \
    $$PWD/sound-effect.cpp \
    $$PWD/trash-cleaned-watcher.cpp


FORMS += \
    $$PWD/connect-server-dialog.ui


