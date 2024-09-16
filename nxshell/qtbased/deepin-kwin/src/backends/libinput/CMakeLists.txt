if (QT_MAJOR_VERSION EQUAL "5")
    qt5_generate_dbus_interface(device.h org.deepin.kwin.InputDevice.xml OPTIONS -A)
else()
    qt_generate_dbus_interface(device.h org.deepin.kwin.InputDevice.xml OPTIONS -A)
endif()

add_custom_target(
    KWinInputDBusInterfaces
    ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.kwin.InputDevice.xml
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.kwin.InputDevice.xml
    DESTINATION
        ${KDE_INSTALL_DBUSINTERFACEDIR}
)

target_sources(kwin PRIVATE
    connection.cpp
    context.cpp
    device.cpp
    events.cpp
    libinput_logging.cpp
    libinputbackend.cpp
)
target_link_libraries(kwin Libinput::Libinput)
