install(FILES
    "dbus/org.lingmo.compositor1.conf"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/dbus-1/system.d"
)

install(FILES
    "dconfig/org.lingmo.Treeland.json"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/dsg/configs/org.lingmo.Treeland"
)

add_subdirectory(systemd)
add_subdirectory(wayland-sessions)
