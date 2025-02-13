set(SCREEN_SRCS
    common/control.cpp
    common/globals.cpp
    common/orientation_sensor.cpp
    common/utils.cpp
    confighandler.cpp
    outputmodel.cpp
    plugin.cpp
    screen.cpp
)

find_package(KF5Screen REQUIRED)
find_package(KF5KIO REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Test Sensors)

add_library(lingmoscreen_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmoscreen_qmlplugins
    Qt5::Core
    Qt5::Quick
    Qt5::Gui
    Qt5::DBus
    Qt5::Sensors
    Qt5::Test

    KF5::Screen
    KF5::KIOCore
)

install(TARGETS lingmoscreen_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/Screen)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/Screen)