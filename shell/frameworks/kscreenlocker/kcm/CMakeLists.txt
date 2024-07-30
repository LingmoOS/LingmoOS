# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_screenlocker\")

set(screenlocker_kcm_SRCS
    kcm.cpp
    kscreenlockerdata.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

qt_add_dbus_interface(screenlocker_kcm_SRCS ../dbus/org.kde.screensaver.xml screenlocker_interface)

kcmutils_add_qml_kcm(kcm_screenlocker SOURCES ${screenlocker_kcm_SRCS})

target_link_libraries(kcm_screenlocker PRIVATE
    settings
    Qt::DBus
    KF6::KCMUtilsCore # KCModuleData
    KF6::KCMUtilsQuick # KQuickManagedConfigModule
    KF6::I18n
    KF6::GlobalAccel
)
