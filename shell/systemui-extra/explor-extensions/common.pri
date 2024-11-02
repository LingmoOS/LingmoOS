INCLUDEPATH += $$PWD
DISTFILES += $$PWD/common.json

exists(/usr/include/lingmosdk/lingmo-com4cxx.h) {
    message("lingmo common for cxx find.")
    DEFINES += LINGMO_COMMON=true
    LIBS += -llingmo-com4cxx
}
