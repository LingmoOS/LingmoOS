# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_screenlocker\")

set(screenlocker_kcm_SRCS
    kcm.cpp
    kscreenlockerdata.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

qt_add_dbus_interface(screenlocker_kcm_SRCS ../dbus/org.kde.screensaver.xml screenlocker_interface)

kcoreaddons_add_plugin(kcm_screenlocker SOURCES ${screenlocker_kcm_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

kcmutils_generate_desktop_file(kcm_screenlocker)
target_link_libraries(kcm_screenlocker
    settings
    Qt::DBus
    KF5::Declarative
    KF5::KCMUtils
    KF5::I18n
    KF5::GlobalAccel
    KF5::Package
    KF5::QuickAddons
    KF5::XmlGui
)

kpackage_install_package(package kcm_screenlocker kcms)
