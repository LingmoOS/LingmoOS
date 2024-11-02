INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/custom-error-handler.h \
    $$PWD/emblem-job.h \
    $$PWD/emblem-provider.h \
    $$PWD/metadata-emblem-provider.h \
    $$PWD/explor-core_global.h           \
    $$PWD/file-info.h                   \
    $$PWD/file-info-job.h               \
    $$PWD/file-info-manager.h           \
    $$PWD/file-enumerator.h             \
    $$PWD/mount-operation.h             \
    $$PWD/file-watcher.h                \
    $$PWD/connect-server-dialog.h       \
    $$PWD/connect-to-server-dialog.h    \
    $$PWD/private/file-vfs-info-private.h \
    $$PWD/volume-manager.h              \
    $$PWD/gerror-wrapper.h              \
    $$PWD/gobject-template.h            \
    $$PWD/file-utils.h                  \
    $$PWD/thumbnail-manager.h           \
    $$PWD/linux-pwd-helper.h            \
    $$PWD/file-meta-info.h              \
    $$PWD/bookmark-manager.h            \
    $$PWD/sync-thread.h                 \
    $$PWD/volumeManager.h               \
    $$PWD/usershare-manager.h           \
    $$PWD/file-infos-job.h              \
    $$PWD/file-vfs-info.h

SOURCES += \
    $$PWD/custom-error-handler.cpp \
    $$PWD/emblem-job.cpp \
    $$PWD/emblem-provider.cpp \
    $$PWD/file-info.cpp                 \
    $$PWD/file-info-job.cpp             \
    $$PWD/file-info-manager.cpp         \
    $$PWD/file-enumerator.cpp           \
    $$PWD/metadata-emblem-provider.cpp \
    $$PWD/mount-operation.cpp           \
    $$PWD/file-watcher.cpp              \
    $$PWD/connect-server-dialog.cpp     \
    $$PWD/connect-to-server-dialog.cpp  \
    $$PWD/private/file-vfs-info-private.cpp \
    $$PWD/volume-manager.cpp            \
    $$PWD/gerror-wrapper.cpp            \
    $$PWD/gobject-template.cpp          \
    $$PWD/file-utils.cpp                \
    $$PWD/thumbnail-manager.cpp         \
    $$PWD/linux-pwd-helper.cpp          \
    $$PWD/file-meta-info.cpp            \
    $$PWD/bookmark-manager.cpp          \
    $$PWD/sync-thread.cpp               \
    $$PWD/volumeManager.cpp		\
    $$PWD/usershare-manager.cpp         \
    $$PWD/file-infos-job.cpp           \
    $$PWD/file-vfs-info.cpp

FORMS += \
    $$PWD/connect-server-dialog.ui
