INCLUDEPATH += $$PWD

#include(../explorer-core.pri)

include(file-operation-dialog/file-operation-dialog.pri)
include(replica/replica.pri)

PKGCONFIG += gio-unix-2.0

HEADERS += \
    $$PWD/file-batch-rename-operation.h \
    $$PWD/file-node.h                           \
    $$PWD/file-operation-helper.h \
    $$PWD/file-operation-internal-dialog.h \
    $$PWD/shared-file-link-operation.h \
    $$PWD/xatom-helper.h                        \
    $$PWD/file-operation.h                      \
    $$PWD/file-node-reporter.h                  \
    $$PWD/file-link-operation.h                 \
    $$PWD/file-copy-operation.h                 \
    $$PWD/file-move-operation.h                 \
    $$PWD/file-trash-operation.h                \
    $$PWD/file-count-operation.h                \
    $$PWD/file-delete-operation.h               \
    $$PWD/file-rename-operation.h               \
    $$PWD/file-operation-manager.h              \
    $$PWD/file-untrash-operation.h              \
    $$PWD/create-template-operation.h           \
    $$PWD/file-operation-progress-bar.h         \
    $$PWD/file-operation-error-dialog.h         \
    $$PWD/file-operation-error-handler.h        \
    $$PWD/file-operation-error-dialogs.h        \
    $$PWD/file-operation-progress-wizard.h      \
    $$PWD/file-operation-error-dialog-base.h    \

SOURCES += \
    $$PWD/file-batch-rename-operation.cpp \
    $$PWD/file-node.cpp                         \
    $$PWD/file-operation-helper.cpp \
    $$PWD/file-operation-internal-dialog.cpp \
    $$PWD/shared-file-link-operation.cpp \
    $$PWD/xatom-helper.cpp                      \
    $$PWD/file-operation.cpp                    \
    $$PWD/file-node-reporter.cpp                \
    $$PWD/file-link-operation.cpp               \
    $$PWD/file-move-operation.cpp               \
    $$PWD/file-copy-operation.cpp               \
    $$PWD/file-trash-operation.cpp              \
    $$PWD/file-count-operation.cpp              \
    $$PWD/file-delete-operation.cpp             \
    $$PWD/file-rename-operation.cpp             \
    $$PWD/file-operation-manager.cpp            \
    $$PWD/file-untrash-operation.cpp            \
    $$PWD/create-template-operation.cpp         \
    $$PWD/file-operation-progress-bar.cpp       \
    $$PWD/file-operation-error-dialog.cpp       \
    $$PWD/file-operation-error-dialogs.cpp      \
    $$PWD/file-operation-progress-wizard.cpp    \
    $$PWD/file-operation-error-dialog-base.cpp  \
