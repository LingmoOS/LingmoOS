include(GNUInstallDirs)

file(GLOB DBusAPI ${CMAKE_CURRENT_LIST_DIR}/dbus/*.xml)

configure_file(
    ${CMAKE_SOURCE_DIR}/api/DDEApplicationManagerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DDEApplicationManagerConfig.cmake
    @ONLY)

install(FILES ${DBusAPI}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dde-application-manager/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DDEApplicationManagerConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DDEApplicationManager/)
