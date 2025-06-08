add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

kcoreaddons_add_plugin(kscreen INSTALL_NAMESPACE "kf5/kded")

target_sources(kscreen PRIVATE
    daemon.cpp
    config.cpp
    output.cpp
    generator.cpp
    device.cpp
    ${CMAKE_SOURCE_DIR}/common/osdaction.cpp
    ${CMAKE_SOURCE_DIR}/common/globals.cpp
    ${CMAKE_SOURCE_DIR}/common/control.cpp
    ${CMAKE_SOURCE_DIR}/common/orientation_sensor.cpp
    ${CMAKE_SOURCE_DIR}/common/utils.cpp
)

ecm_qt_declare_logging_category(kscreen HEADER kscreen_daemon_debug.h IDENTIFIER KSCREEN_KDED CATEGORY_NAME kscreen.kded DESCRIPTION "kscreen kded (kscreen)" EXPORT KSCREEN)

qt_add_dbus_interface(dbus_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt_add_dbus_interface(dbus_SRCS
    ../osd/org.kde.kscreen.osdService.xml
    osdservice_interface)
qt_add_dbus_adaptor(dbus_SRCS
    org.kde.KScreen.xml
    daemon.h
    KScreenDaemon
)

target_sources(kscreen PRIVATE ${dbus_SRCS})

target_link_libraries(kscreen PRIVATE
                              Qt::Widgets
                              Qt::DBus
                              Qt::Quick
                              Qt::Sensors
                              KF5::Declarative
                              KF5::Screen
                              KF5::DBusAddons
                              KF5::I18n
                              KF5::XmlGui
                              KF5::GlobalAccel
                              KF5::ScreenDpms
)

if(X11_FOUND)
    target_link_libraries(kscreen PRIVATE X11::X11 X11::Xi X11::XCB XCB::ATOM)
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(kscreen PRIVATE Qt5::X11Extras)
    else()
        target_link_libraries(kscreen PRIVATE Qt::GuiPrivate)
    endif()
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.json
               @ONLY)
