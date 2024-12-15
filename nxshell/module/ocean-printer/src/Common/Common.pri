QT += network sql

INCLUDEPATH +=  \
                $$PWD/../cppcups \
                /usr/include/samba-4.0/

HEADERS +=  \
    $$PWD/common.h \
    $$PWD/qtconvert.h \
    $$PWD/zsettings.h \
    $$PWD/cupsconnectionfactory.h \
    $$PWD/ztaskinterface.h \
    $$PWD/zjobmanager.h \
    $$PWD/cupsattrnames.h \
    $$PWD/config.h \
    $$PWD/reviselogger.h \
    $$PWD/vendor/addprinter_p.h \
    $$PWD/vendor/addprinter.h \
    $$PWD/vendor/printerservice.h \
    $$PWD/vendor/zdevicemanager.h \
    $$PWD/vendor/zdrivermanager_p.h \
    $$PWD/vendor/zdrivermanager.h \
    $$PWD/vendor/ztroubleshoot_p.h \
    $$PWD/vendor/ztroubleshoot.h

SOURCES +=  \
    $$PWD/common.cpp \
    $$PWD/qtconvert.cpp \
    $$PWD/zsettings.cpp \
    $$PWD/cupsconnectionfactory.cpp \
    $$PWD/ztaskinterface.cpp \
    $$PWD/zjobmanager.cpp \
    $$PWD/reviselogger.cpp \
    $$PWD/vendor/addprinter.cpp \
    $$PWD/vendor/printerservice.cpp \
    $$PWD/vendor/zdevicemanager.cpp \
    $$PWD/vendor/zdrivermanager.cpp \
    $$PWD/vendor/ztroubleshoot.cpp
