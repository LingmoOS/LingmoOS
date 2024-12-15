TEMPLATE = lib
TARGET = dframeworkdbus
VERSION = 2.0
CONFIG += c++11 create_pc create_prl no_install_prl
CONFIG += no_keywords

# 增加安全编译参数
QMAKE_CFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy

load(dtk_qmake)
include(qtdbusextended/qtdbusextended.pri)

HEADERS += \
    types/arrayint.h \
    types/zoneinfo.h \
    types/audioport.h \
    types/audioportlist.h \
    types/batterypercentageinfo.h \
    types/batterypresentinfo.h \
    types/batterystateinfo.h \
    types/brightnessmap.h \
    types/touchscreenmap.h \
    types/touchscreeninfolist.h \
    types/touchscreeninfolist_v2.h \
    types/screenrect.h \
    types/inputdevicelist.h \
    types/keyboardlayoutlist.h \
    types/localelist.h \
    types/mfainfolist.h \
    types/arealist.h \
    types/qvariantlist.h \
    types/wacomdevicelist.h \
    types/searchmap.h \
    types/resolutionlist.h \
    types/resolution.h \
    types/rotationlist.h \
    types/reflectlist.h \
    types/appupdateinfolist.h \
    types/mirrorinfolist.h \
    types/windowlist.h \
    types/networkerrors.h \
    types/networkconnectiondata.h \
    types/networkavailablekeys.h \
    types/propertylist.h \
    types/dbusobjectpathlist.h \
    types/linkinfolist.h \
    types/sinkinfolist.h \
    types/qvariantmap.h \
    types/appscgroupinfo.h \
    types/appscgroupinfolist.h \
    types/chrometablist.h \
    types/windowinfomap.h \
    types/dbusimagelist.h \
    types/dbustooltip.h \
    types/dockrect.h \
    types/intstring.h \
    types/controlcenterrect.h \
    types/defenderversioninfo.h \
    types/defenderfileinfo.h \
    types/defenderupdatestate.h \
    types/defenderdissection.h \
    types/launcheriteminfo.h \
    types/launcheriteminfolist.h \
    types/hardwareinfo.h \
    types/defenderprocesslist.h \
    types/defenderprocessinfo.h \
    types/defenderdisplay.h \
    types/defenderappflow.h \
    types/defenderprocinfo.h \
    types/defenderprocinfolist.h \
    types/policysubject.h \
    types/policyresult.h \
    types/policydetails.h \
    types/lastoreupdatepackagesinfo.h

SOURCES += \
    types/arrayint.cpp \
    types/zoneinfo.cpp \
    types/screenrect.cpp \
    types/inputdevicelist.cpp \
    types/localelist.cpp \
    types/mfainfolist.cpp \
    types/audioport.cpp \
    types/audioportlist.cpp \
    types/keyboardlayoutlist.cpp \
    types/arealist.cpp \
    types/qvariantlist.cpp \
    types/brightnessmap.cpp \
    types/touchscreenmap.cpp \
    types/touchscreeninfolist.cpp \
    types/touchscreeninfolist_v2.cpp \
    types/batterypresentinfo.cpp \
    types/batterystateinfo.cpp \
    types/batterypercentageinfo.cpp \
    types/wacomdevicelist.cpp \
    types/searchmap.cpp \
    types/resolutionlist.cpp \
    types/resolution.cpp \
    types/rotationlist.cpp \
    types/reflectlist.cpp \
    types/appupdateinfolist.cpp \
    types/mirrorinfolist.cpp \
    types/windowlist.cpp \
    types/networkerrors.cpp \
    types/networkconnectiondata.cpp \
    types/networkavailablekeys.cpp \
    types/propertylist.cpp \
    types/dbusobjectpathlist.cpp \
    types/linkinfolist.cpp \
    types/sinkinfolist.cpp \
    types/qvariantmap.cpp \
    types/appscgroupinfolist.cpp \
    types/chrometablist.cpp \
    types/windowinfomap.cpp \
    types/dbusimagelist.cpp \
    types/dbustooltip.cpp \
    types/dockrect.cpp \
    types/intstring.cpp \
    types/controlcenterrect.cpp \
    types/defenderversioninfo.cpp \
    types/defenderfileinfo.cpp \
    types/defenderupdatestate.cpp \
    types/defenderdissection.cpp \
    types/launcheriteminfo.cpp \
    types/launcheriteminfolist.cpp \
    types/hardwareinfo.cpp \
    types/defenderprocesslist.cpp \
    types/defenderprocessinfo.cpp \
    types/defenderdisplay.cpp \
    types/defenderappflow.cpp \
    types/defenderprocinfo.cpp \
    types/defenderprocinfolist.cpp \
    types/policysubject.cpp \
    types/policyresult.cpp \
    types/policydetails.cpp \
    types/lastoreupdatepackagesinfo.cpp

!system(python3 $$PWD/generate_code.py): error("Failed to generate code")
include(generated/generated.pri)

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$INSTALL_ROOT/usr/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}

includes.files =
includes.path = $$INSTALL_ROOT/usr/include/libdframeworkdbus-$$VERSION

cmake.input      = $$PWD/DFrameworkdbusConfig.in
cmake.output     = $$OUT_PWD/DFrameworkdbusConfig.cmake

QMAKE_SUBSTITUTES += cmake

isEmpty(LIB_INSTALL_DIR) {
    cmake_config.path = $$INSTALL_ROOT/usr/lib/cmake/DFrameworkdbus/
} else {
    cmake_config.path = $$LIB_INSTALL_DIR/cmake/DFrameworkdbus/
}

cmake_config.files = $$OUT_PWD/DFrameworkdbusConfig.cmake

type_headers.files =
type_headers.path = $$includes.path/types

for(header, HEADERS) {
    TMP = $${dirname(header)}
    DIRNAME = $${basename(TMP)}

    equals(DIRNAME, types) {
        eval(type_headers.files += $$header)
    } else {
        eval(includes.files += $$header)
    }
}

host_sw_64 {
    QMAKE_CXXFLAGS += -mieee
}

QMAKE_PKGCONFIG_NAME = libdframeworkdbus
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESCRIPTION = libdframeworkdbus
QMAKE_PKGCONFIG_INCDIR = $$includes.path
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += includes type_headers target cmake_config
