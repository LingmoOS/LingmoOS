VERSION = 3.2.2
DEFINES += VERSION='\\"$${VERSION}\\"'
DEFINES += MULTI_DISABLE

DEFINES += V10_SP1='\\"V10SP1\\"'
DEFINES += V10_SP1_EDU='\\"V10SP1-edu\\"'

DEB_VERSION = 3.14.5.7
DEFINES += DEB_VERSION='\\"$${DEB_VERSION}\\"'

exists(/usr/include/lingmosdk/lingmo-com4cxx.h) {
    message("lingmo common for cxx find.")
    DEFINES += LINGMO_COMMON=true
    LIBS += -llingmo-com4cxx
}

exists(/usr/include/lingmosdk/applications/kdialog.h) {
    message("kyfiledialog find")
    DEFINES += LINGMO_FILE_DIALOG
}

exists(/usr/include/lingmosdk/lingmosdk-system/libkysysinfo.h) {
    message("kysysinfo find")
    DEFINES += LINGMO_SDK_SYSINFO
}

exists(/usr/include/lingmosdk/applications/kaboutdialog.h) {
    message("kyqtwidgets find")
    DEFINES += LINGMO_SDK_QT_WIDGETS
}

exists(/usr/include/lingmosdk/applications/lingmostylehelper/lingmostylehelper.h) {
    message("kywaylandhelper find")
    DEFINES += LINGMO_SDK_WAYLANDHELPER
}

exists("/usr/include/libkyudfburn/udfburn_global.h") {
    DEFINES += LINGMO_UDF_BURN
}

exists("/usr/include/lingmo-search/libsearch_global.h") {
    DEFINES += LINGMO_LINGMO_SEARCH
}

exists("/usr/include/lingmosdk/desktop/lingmosdk-soundeffects_global.h") {
    DEFINES += LINGMO_SDK_SOUND_EFFECTS
    PKGCONFIG += lingmosdk-soundeffects
}

exists("/usr/include/lingmosdk/lingmosdk-system/libkydate.h") {
    DEFINES += LINGMO_SDK_DATE
    PKGCONFIG += lingmosdk-systime
}

exists(/usr/include/lingmosdk/lingmosdk-system/libkysysinfo.h) {
    message("kysysinfo find")
    DEFINES += LINGMO_SDK_SYSINFO
}

exists(/usr/include/lingmosdk/diagnosetest/libkydatacollect.h) {
    message("diagnosetest find")
    DEFINES += LINGMO_SDK_DATACOLLECT
}
#include <lingmosdk/applications/kabase/log.hpp>
exists(/usr/include/applications/kabase/log.hpp) {
    message ("lingmosdkbase found")
    DEFINES += LINGMO_SDK_KABASE
}

USE_SET_ICON=$$(USE_SET_ICON_GEOMETRY)
equals(USE_SET_ICON,1) {
    message("USE_SET_ICON_GEOMETRY is set")
    DEFINES += KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
} else {
    message("USE_SET_ICON_GEOMETRY is not set")
}

DEFINES += VFS_CUSTOM_PLUGIN
