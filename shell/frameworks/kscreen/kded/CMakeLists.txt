add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

kcoreaddons_add_plugin(kscreen INSTALL_NAMESPACE "kf6/kded")

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

target_sources(kscreen PRIVATE ${dbus_SRCS})

target_link_libraries(kscreen PRIVATE
                              Qt::Widgets
                              Qt::DBus
                              Qt::Quick
                              Qt::Sensors
                              KF6::CoreAddons
                              KF6::Screen
                              KF6::DBusAddons
                              KF6::I18n
                              KF6::XmlGui
)

if(WITH_X11)
    target_link_libraries(kscreen PRIVATE Qt::GuiPrivate X11::X11 X11::Xi X11::XCB XCB::ATOM)
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.json
               @ONLY)
