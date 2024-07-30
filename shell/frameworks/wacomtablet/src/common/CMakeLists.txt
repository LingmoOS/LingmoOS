add_definitions(-DTRANSLATION_DOMAIN=\"wacomtablet\")

set(wacom_common_LIBS
  Qt::Core
  Qt::Gui
  Qt::Widgets
  KF6::I18n
  KF6::GlobalAccel
  KF6::ConfigCore
  KF6::CoreAddons
  XCB::XINPUT
  X11::Xi
  PkgConfig::LIBWACOM
)

if(QT_MAJOR_VERSION EQUAL "5")
  list(APPEND wacom_common_LIBS Qt::X11Extras)
else()
  list(APPEND wacom_common_LIBS Qt::GuiPrivate)
endif()

set(wacom_common_SRC
    aboutdata.cpp
    buttonshortcut.cpp
    dbustabletinterface.cpp
    deviceinformation.cpp
    deviceprofile.cpp
    deviceprofileconfigadaptor.cpp
    deviceprofiledefaults.cpp
    deviceproperty.cpp
    devicetype.cpp
    libwacomwrapper.cpp
    mainconfig.cpp
    profilemanager.cpp
    profilemanagement.cpp
    property.cpp
    propertyadaptor.cpp
    screenrotation.cpp
    screenmap.cpp
    screensinfo.cpp
    screenspace.cpp
    stringutils.cpp
    tabletarea.cpp
    tabletdatabase.cpp
    tabletinfo.cpp
    tabletinformation.cpp
    tabletprofile.cpp
    tabletprofileconfigadaptor.cpp
    x11input.cpp
    x11inputdevice.cpp
    x11wacom.cpp

    aboutdata.h
    buttonshortcut.h
    dbustabletinterface.h
    deviceinformation.h
    deviceprofile.h
    deviceprofileconfigadaptor.h
    deviceprofiledefaults.h
    deviceproperty.h
    devicetype.h
    libwacomwrapper.h
    mainconfig.h
    profilemanager.h
    profilemanagement.h
    property.h
    propertyadaptor.h
    screenrotation.h
    screenmap.h
    screensinfo.h
    screenspace.h
    stringutils.h
    tabletarea.h
    tabletdatabase.h
    tabletinfo.h
    tabletinformation.h
    tabletprofile.h
    tabletprofileconfigadaptor.h
    x11input.h
    x11inputdevice.h
    x11wacom.h
)

ecm_qt_declare_logging_category(
    wacom_common_SRC
    HEADER "logging.h"
    DESCRIPTION "wacomtablet (common)"
    IDENTIFIER "COMMON"
    CATEGORY_NAME "org.kde.wacomtablet.common"
    EXPORT WACOMTABLET
)

qt_add_dbus_interfaces(wacom_common_SRC
                        ../kded/org.kde.Wacom.xml)

add_library( wacom_common STATIC ${wacom_common_SRC})

target_link_libraries( wacom_common ${wacom_common_LIBS} )
set_target_properties( wacom_common PROPERTIES COMPILE_FLAGS "-fPIC" )
